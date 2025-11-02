#pragma once

#include <core/boost_include>
#include <nlohmann/json.hpp>

struct response_manager {
    using response_t = beast::http::response<beast::http::string_body>;
    using response_pointer_t = std::shared_ptr<response_t>;

    static inline const beast::http::status default_status = beast::http::status::ok;

    static inline const std::unordered_map<std::string, std::string> mime_types = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".txt", "text/plain"}
    };

    static std::string get_mime_type(const std::string& extension, const std::string& default_ = "application/octet-stream") {
        auto it = mime_types.find(extension);
        return (it == mime_types.end()) ? default_ : it->second;
    }

    static auto make_response(unsigned int version, const std::string& server_name) {
        auto response = std::make_shared<response_t>();
        response->version(version);
        response->set(beast::http::field::server, server_name);
        return response;
    }

    static void edit_response(response_pointer_t response, const std::string& content_type, const std::string& body,
            beast::http::status result) {
        response->set(beast::http::field::content_type, content_type);
        response->body() = body;
        response->result(result);
        response->prepare_payload();
    }

    static void check_method(response_pointer_t response, const std::string& expected_method) {
        
    }

    static void send_json(response_pointer_t response, const nlohmann::json& json,
            beast::http::status result = default_status) {
        edit_response(response, "application/json", json.dump(), result);
    }
};
