#pragma once

#include <iostream>

#include <core/http_router>
#include <core/https_server>
#include <core/database>
#include <core/authorization_service>
#include <core/marketplace>

class application {
private:
    using response_t = http_router<https_server>::response_t;
    using response_pointer_t = http_router<https_server>::response_pointer_t;

    using request_t = http_router<https_server>::request_t;
    using request_pointer_t = http_router<https_server>::request_pointer_t;

    asio::io_context io_context;
    http_router<https_server> server;

    pqxx_wrapper db; // pqxx_wrapper = database

    authorization_service auth_service;

    marketplace market;

    void no_route_invoker(
        request_pointer_t request,
        response_pointer_t response
    );

public:
    application(
        const std::string& postgres_setting,
        config_t<authorization_service> auth_service_config,
        config_t<https_server> server_config,
        config_t<marketplace> marketplace_config
    );
    
    application(const application&) = delete;
    application& operator=(const application&) = delete;
    
    int execute();
    void stop();
};