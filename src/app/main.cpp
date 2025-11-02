#include <fstream>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "./application.h"

std::string read_file(std::ifstream& file) {
    std::string data, line;
    while (std::getline(file, line))
        data += line + "\n";
    return data;
}

std::unique_ptr<application> app = nullptr;

void signal_handler(int signal) {
    spdlog::critical("Received {} signal!", signal);
    spdlog::debug("Try to stop application!");
    app->stop();
};

int create_application(const char* application_setting_filename) {
    using json = nlohmann::json;
    // open & check json file
    std::ifstream application_setting_file(application_setting_filename, std::ios_base::in);
    if (!application_setting_file.is_open()) {
        spdlog::critical("Unable to open {}!", application_setting_filename);
        return 1;
    }

    std::string application_setting_data = read_file(application_setting_file);
    
    // check json
    if (!json::accept(application_setting_data)) {
        spdlog::critical("Incorrect JSON file: {}!", application_setting_filename);
        return 1;
    }

    auto application_setting = json::parse(application_setting_data);
    if (!json_manager::contains(application_setting, "postgres", "server", "authorization_service")) {
        spdlog::critical("Incorrect JSON file: {}!", application_setting_filename);
        return 1;
    }

    // check json[server]
    auto server_setting = application_setting["server"];
    if (!(json_manager::contains(server_setting, "port", "certificate_chain_file", "private_key_file", "CORS") 
            && server_setting.at("port").is_number())) {
        spdlog::critical("Incorrect JSON file: {}!", application_setting_filename);
        return 1;
    }
    
    // check json[server][CORS]
    auto CORS_setting = server_setting["CORS"];
    if (!json_manager::contains(CORS_setting, "Origin", "Methods", "Headers")) {
        spdlog::critical("Incorrect JSON file: {}!", application_setting_filename);
        return 1;
    }

    // check json[postgres]
    auto postgres_setting = application_setting["postgres"];
    if (!json_manager::contains(postgres_setting, "setting")) {
        spdlog::critical("Incorrect JSON file: {}!", application_setting_filename);
        return 1;
    }

    // check json[authorization_service]
    auto authorization_service_setting = application_setting["authorization_service"];
    if (!json_manager::contains(authorization_service_setting, "user_table", "user_logs_table", "jwt_server_name")) {
        spdlog::critical("Incorrect JSON file: {}!", application_setting_filename);
        return 1;
    }

    // check json[marketplace]
    auto marketplace_setting = application_setting["marketplace"];
    if (!json_manager::contains(marketplace_setting, "categories_table")) {
        spdlog::critical("Incorrect JSON file: {}!", application_setting_filename);
        return 1;
    }

    // create & config application
    app = std::make_unique<application>(
        postgres_setting["setting"],
        config_t<authorization_service> {
            authorization_service_setting["user_table"],
            authorization_service_setting["user_logs_table"],
            authorization_service_setting["jwt_server_name"]
        },
        config_t<https_server> {
            server_setting["port"],
            server_setting["certificate_chain_file"],
            server_setting["private_key_file"],
            server_setting["server_name"],
            CORS_config_t {
                CORS_setting["Origin"],
                CORS_setting["Methods"],
                CORS_setting["Headers"]
            }
        },
        config_t<marketplace> {
            marketplace_setting["categories_table"]
        }
    );

    return 0;
}

int main() {
    if (int result = create_application("application-setting.json"); result != 0)
        return result;

    // add signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // run application
    return app->execute();
}