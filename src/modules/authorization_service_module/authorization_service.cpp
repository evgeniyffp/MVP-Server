#include "./authorization_service.h"

#include <fmt/format.h>

#include "./jwt_manager.h"
#include "./password_manager.h"
#include "./role_manager.h"

authorization_service::authorization_service(database& db, config_t<authorization_service> config)
    : db(db), config(std::move(config)) {}

std::expected<std::string, std::string> authorization_service::register_(std::string json_data) {
    auto [data, is_valid] = json_manager::create(json_data, "username", "email", "password", "role");
    if (!is_valid) {
        return std::unexpected("Invalid json!");
    }

    auto transaction = db.get_transaction();

    std::string username = data["username"];
    if (!is_valid_username(username))
        return std::unexpected("Incorrect username!");
    if (!transaction.exec(fmt::format("SELECT * FROM {} WHERE username = $1;", config.user_table_name), pqxx::params(username)).empty()) {
        transaction.abort();
        return std::unexpected("Username is already in use!");
    }

    std::string email = data["email"];
    if (!is_valid_email(email))
        return std::unexpected("Incorrect email!");
    if (!transaction.exec(fmt::format("SELECT * FROM {} WHERE email = $1;", config.user_table_name), pqxx::params(email)).empty()) {
        transaction.abort();
        return std::unexpected("Email is already in use!");
    }

    std::string password = data["password"];
    switch (password_manager::check_complexity(password)) {
        case password_manager::complexity_e::invalid:
            return std::unexpected("Password is invalid!");
        case password_manager::complexity_e::easy:
            return std::unexpected("Password is too easy!");
        default: break;
    }
    std::string password_hash = password_manager::hash(password);

    std::string role = data["role"];
    if (role_manager::check(role) == role_manager::role_e::invalid) {
        return std::unexpected("Role is invalid!");
    }

    try {
        constexpr auto sql = "INSERT INTO {}(username, email, password_hash, role) VALUES($1, $2, $3, $4);";
        transaction.exec(
            fmt::format(sql, config.user_table_name), 
            pqxx::params(username, email, password_hash, role)
        );
    }
    catch (const std::exception& e) {
        transaction.abort();
        return std::unexpected("Incorrect data!");
    }

    try {
        constexpr auto sql = "INSERT INTO {}(user_id, log_event) VALUES((SELECT id FROM {} WHERE username=$1), $2);";
        auto log = "Have been registered and received JWT token.";
        transaction.exec(fmt::format(sql, config.user_logs_table_name, config.user_table_name), pqxx::params(username, log));
        transaction.commit();
    }
    catch (const std::exception& e) {
        transaction.abort();
        return std::unexpected("Unknown error!");
    }

    return jwt_manager::create_token(username, config.server_name, std::getenv("JWT_SECRET"));
}

std::expected<std::string, std::string> authorization_service::login(std::string json_data) {
    auto [data, is_valid] = json_manager::create(json_data, "username", "password");
    if (!is_valid) {
        return std::unexpected("Invalid json!");
    }

    auto transaction = db.get_transaction();

    std::string username = data["username"];
    if (!is_valid_username(username))
        return std::unexpected("Incorrect username!");

    if (transaction.exec(fmt::format("SELECT * FROM {} WHERE username = $1;", config.user_table_name), pqxx::params(username)).empty()) {
        transaction.abort();
        return std::unexpected("Username is not in use!");
    }

    std::string password_hash = transaction.exec(fmt::format("SELECT password_hash FROM {} WHERE username = $1;", config.user_table_name), pqxx::params(username))[0][0].c_str();

    if (!password_manager::verify(data["password"], password_hash)) {
        return std::unexpected("Password is incorrect!");
    }

    try {
        constexpr auto sql = "INSERT INTO {}(user_id, log_event) VALUES((SELECT id FROM {} WHERE username=$1), $2);";
        auto log = "Have been logged and received JWT token.";
        transaction.exec(fmt::format(sql, config.user_logs_table_name, config.user_table_name), pqxx::params(username, log));
        transaction.commit();
    }
    catch (const std::exception& e) {
        transaction.abort();
        return std::unexpected("Unknown error!");
    }

    return jwt_manager::create_token(username, config.server_name, std::getenv("JWT_SECRET"));
}

bool authorization_service::verify_token(const std::string& token) noexcept {
    return jwt_manager::verify_token(token, config.server_name, std::getenv("JWT_SECRET"));
}