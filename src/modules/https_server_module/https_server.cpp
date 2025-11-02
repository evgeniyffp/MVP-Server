#include "./https_server.h"

#include <spdlog/spdlog.h>

https_server::https_server(config_t<https_server> config, asio::io_context& io_context, std::function<invoker_t> invoker)
    : config(std::move(config))
    , context_(ssl::context::tlsv13)
    , acceptor_(io_context, tcp::endpoint(tcp::v4(), config.port))
    , invoker(std::move(invoker)) {
        
    // Configuring the SSL context
    context_.set_options(
        ssl::context::default_workarounds |
        ssl::context::no_sslv2 |
        ssl::context::single_dh_use);
        
    // Installing the certificate and private key
    context_.use_certificate_chain_file(this->config.certificate_chain_file);
    context_.use_private_key_file(this->config.private_key_file, ssl::context::pem);
        
    // Start accepting connections
    do_accept();
}

void https_server::do_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                // Create an SSL socket and perform a handshake
                auto ssl_socket = std::make_shared<ssl::stream<tcp::socket>>(
                    std::move(socket), context_);
                
                ssl_socket->async_handshake(
                    ssl::stream_base::server,
                    [this, ssl_socket](boost::system::error_code ec) {
                        if (!ec) {
                            // After a successful handshake, we process the connection
                            handle_connection(ssl_socket);
                        }
                        else if (ec == boost::asio::ssl::error::stream_truncated) {
                            // Do nothing
                            // This is not a bug, but a feature of TLS.
                            // Many browsers and HTTP libraries do not perform a full TLS shutdown for the sake of speed.
                        }
                        else {
                            spdlog::error("Handshake failed: {}.", ec.message());
                        }
                    }
                );
            }
                
            // Accept the following connection
            do_accept();
        }
    );
}

void https_server::handle_connection(std::shared_ptr<ssl::stream<tcp::socket>> ssl_socket) {
    auto buffer = std::make_shared<beast::flat_buffer>();
    auto request = std::make_shared<request_t>();
        
    // Reading the HTTP request
    beast::http::async_read(
        *ssl_socket,
        *buffer,
        *request,
        [this, ssl_socket, buffer, request](boost::system::error_code ec, size_t request_body_size) {
            if (ec) {
                spdlog::error("Read HTTP request failed: {}.", ec.message());
                return;
            }
            
            spdlog::info("Received: method - {}, target - {}, body - {}.\n", 
                request->method_string(), request->target(), request->body());

            auto response = response_manager::make_response(request->version(), config.server_name);
            if (request_body_size > config.max_request_body_size) {
                std::string body = fmt::format("Request body size (which is {}) > config.max_request_body_size(which is {}).", 
                    request_body_size, config.max_request_body_size);
                spdlog::error(body);
                response_manager::edit_response(response, "text/plain", body, beast::http::status::bad_request);
            }
            else {
                invoker(request, response);
            }
            
            if (config.CORS_config.has_value()) {
                auto CORS_config = config.CORS_config.value();
                response->set("Access-Control-Allow-Origin", CORS_config.origin);
                response->set("Access-Control-Allow-Methods", CORS_config.methods);
                response->set("Access-Control-Allow-Headers", CORS_config.headers);   
            }
                
            spdlog::info("Sended: result - {}, body - {}.\n", response->result_int(), response->body());

            // Sending a response
            beast::http::async_write(
                *ssl_socket,
                *response,
                [ssl_socket, response](boost::system::error_code ec, size_t) {
                    // Closing the connection after sending the response
                    if (ec) {
                        spdlog::error("Write failed: {}.", ec.message());
                    }
                }
            );
        }
    );
}