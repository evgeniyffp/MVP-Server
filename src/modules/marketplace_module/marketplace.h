#pragma once

#include <string>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <core/utils>
#include <core/http_router>

class marketplace;

template <>
struct config_t<marketplace> {
    std::string categories_table;
};

class marketplace {
private:
    database& db;
    config_t<marketplace> config;

public:
    marketplace(database& db, config_t<marketplace> config)
        : db(db)
        , config(std::move(config)) {}

    template <typename https_server_t>
    void set_routes(http_router<https_server_t>& server) {
        using response_pointer_t = http_router<https_server>::response_pointer_t;
        using request_pointer_t = http_router<https_server>::request_pointer_t;

        server.set("/api/categories", [this](request_pointer_t request, response_pointer_t response) {
            if (request->method_string() != "GET") {
                response_manager::edit_response(response, "text/plain", "Incorrect request!", beast::http::status::bad_request);
                return;
            }

            auto transaction = db.get_transaction();
            try {
                nlohmann::json data;
                constexpr auto sql = "SELECT id, name, slug FROM {};";
                for (auto [id, name, slug] : transaction.query<int, std::string, std::string>(fmt::format(sql, config.categories_table))) {
                    constexpr auto sql = "SELECT parent_id FROM {} WHERE id = $1;";
                    auto result = transaction.exec(fmt::format(sql, config.categories_table), pqxx::params(id))[0][0];
                    auto id_str = std::to_string(id);
                    if (result.is_null()) {
                        data[id_str]["name"] = name;
                        data[id_str]["slug"] = slug;
                    }
                    else {
                        auto parent_id = result.as<std::string>();
                        data[parent_id]["subcategories"][id_str]["name"] = name;
                        data[parent_id]["subcategories"][id_str]["slug"] = slug;
                    }
                }

                response_manager::send_json(response, data);
            }
            catch (const std::exception& e) {
                spdlog::error(e.what());
                response_manager::edit_response(response, "text/plain", "Unknown error!", beast::http::status::bad_request);
            }
        });
    }
};
