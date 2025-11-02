#include "./application.h"

#include <vector>

#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include "../modules/authorization_service_module/jwt_manager.h"

// invoker_t
void application::no_route_invoker(
    request_pointer_t request,
    response_pointer_t response
) {
    std::string body = fmt::format("There are no route for target {} and method {}.",
        request->target(), request->method_string());
    spdlog::error(body);
    response_manager::edit_response(response, "text/plain", body, beast::http::status::bad_request);
}

application::application(
    const std::string& postgres_setting,
    config_t<authorization_service> auth_service_config,
    config_t<https_server> server_config,
    config_t<marketplace> marketplace_config
)   : io_context()
    , server(std::move(server_config), io_context,
        std::bind(&application::no_route_invoker, this, std::placeholders::_1, std::placeholders::_2))
    , db(postgres_setting)
    , auth_service(db, std::move(auth_service_config))
    , market(db, std::move(marketplace_config)) {}

int application::execute() try {
    server.set_directory("/client/", "./client/");
    
    server.set_redirection("/", "/client/index.html");

    market.set_routes(server);

    server.set("/api/register", [this](request_pointer_t request, response_pointer_t response){
        if (!(request->method_string() == "POST" && (*request)[beast::http::field::content_type] == "application/json")) {
            response_manager::edit_response(response, "text/plain", "Incorrect request!", beast::http::status::bad_request);
            return;
        }
        
        if (auto jwt_token_ex = auth_service.register_(request->body()); jwt_token_ex.has_value()) {
            response_manager::edit_response(response, "text/plain", *jwt_token_ex, beast::http::status::ok);
        }
        else {
            response_manager::edit_response(response, "text/plain", jwt_token_ex.error(), beast::http::status::bad_request);
        }
    });

    server.set("/api/login", [this](request_pointer_t request, response_pointer_t response){
        if (!(request->method_string() == "POST" && (*request)[beast::http::field::content_type] == "application/json")) {
            response_manager::edit_response(response, "text/plain", "Incorrect request!", beast::http::status::bad_request);
            return;
        }
        
        if (auto jwt_token_ex = auth_service.login(request->body()); jwt_token_ex.has_value()) {
            response_manager::edit_response(response, "text/plain", *jwt_token_ex, beast::http::status::ok);
        }
        else {
            response_manager::edit_response(response, "text/plain", jwt_token_ex.error(), beast::http::status::bad_request);
        }
    });
    
    server.set("/api", [](request_pointer_t request, response_pointer_t response){
        response_manager::edit_response(response, "text/plain", 
            "use /api/register to register: method - POST, conted_type - application/json, body - \n"
            "{\n\t\"username\":<username>,\n\t\"email\":<email>,\n\t\"password\":<password>\n}\n",
            beast::http::status::ok
        );
    });

    server.set("/health/postgres", [this](request_pointer_t, response_pointer_t response) {
        try {
            auto transaction = db.get_transaction();
            transaction.exec("SELECT 1;");
            transaction.commit();
            
            response_manager::edit_response(response, "text/plain", "Requests are being executed", beast::http::status::ok);
        }
        catch (...) {
            response_manager::edit_response(response, "text/plain", "Requests aren not being executed", beast::http::status::service_unavailable);
        }
    });

    server.set("/health/application", [](request_pointer_t request, response_pointer_t response) {
        response_manager::edit_response(response, "text/plain", "Requests are being executed", beast::http::status::ok);
    });

    server.set("/health", [this](request_pointer_t request, response_pointer_t response) {
        bool is_all_healthy = true;

        nlohmann::json info;
        std::vector<std::string> services = { "application", "postgres" };

        for (const auto& service : services) {
            server.get(std::string("/health/") + service).value()(request, response);
            if (response->result() == beast::http::status::service_unavailable)
                is_all_healthy = false;
            info[service]["info"] = response->body();
            info[service]["result"] = response->result();
        }

        nlohmann::json body;
        body["info"] = info;

        response_manager::edit_response(response, "application/json", body.dump(), 
            is_all_healthy ? beast::http::status::ok : beast::http::status::service_unavailable);
    });
    
    server.set("/ping", [](request_pointer_t request, response_pointer_t response){
        response_manager::edit_response(response, "text/plain", "pong", beast::http::status::ok);
    });

    spdlog::info("SSL server listening on port 8443...");
    io_context.run();
    spdlog::info("SSL server stoped.");
    
    return 0;
}
catch (const std::exception& e) {
    spdlog::critical("{}", e.what());
    return 1;
}

void application::stop() {
    io_context.stop();
}