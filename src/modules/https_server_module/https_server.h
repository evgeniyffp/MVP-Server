#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <optional>

#include <core/utils>
#include <core/response_manager>

struct CORS_config_t {
    std::string origin;
    std::string methods;
    std::string headers;
};

class https_server;

template <>
struct config_t<https_server> {
    unsigned short port;
    std::string certificate_chain_file;
    std::string private_key_file;
    std::string server_name;
    std::optional<CORS_config_t> CORS_config;
    unsigned int max_request_body_size = 256 * 8; // in bytes
};

class https_server {
public:
    using response_t = response_manager::response_t;
    using response_pointer_t = response_manager::response_pointer_t;

    using request_t = beast::http::request<beast::http::string_body>;
    using request_pointer_t = std::shared_ptr<request_t>;

    using invoker_t = void(request_pointer_t, response_pointer_t);

    // invoker will be called when a correct request is received from the client
    https_server(config_t<https_server> config, asio::io_context& io_context, std::function<invoker_t> invoker);

private:
    config_t<https_server> config;
    ssl::context context_;
    tcp::acceptor acceptor_;
    std::function<invoker_t> invoker;

    void do_accept();

    void handle_connection(std::shared_ptr<ssl::stream<tcp::socket>> ssl_socket);
};