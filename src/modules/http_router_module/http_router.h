#pragma once

#include <unordered_map>
#include <optional>
#include <filesystem>
#include <fstream>

#include <core/boost_include>
#include <core/utils>
#include <core/response_manager>

#include <spdlog/spdlog.h>

template <class http_server_t>
class http_router {
public:
    using response_t = http_server_t::response_t;
    using response_pointer_t = http_server_t::response_pointer_t;

    using request_t = http_server_t::request_t;
    using request_pointer_t = http_server_t::request_pointer_t;

    using invoker_t = typename http_server_t::invoker_t;
    using invoker_function_t = std::function<invoker_t>;

    http_router(config_t<http_server_t> config, asio::io_context& io_context, invoker_function_t no_route_invoker)
        : server(std::move(config), io_context, std::bind(&http_router::invoker, this, std::placeholders::_1, std::placeholders::_2))
        , no_route_invoker(std::move(no_route_invoker)) {}

    std::optional<invoker_function_t> set(const std::string& target, invoker_function_t invoker) {
        auto [it, is_correct] = routes.insert({target, std::move(invoker)});
        if (is_correct)
            return it->second;
        else 
            return std::nullopt;
    }

    std::optional<invoker_function_t> get(const std::string& request_target) const {
        for (auto& [target, invoker] : routes) {
            if (target_equals(target, request_target)) {
                return invoker;
            }
        }
        return std::nullopt;
    }

    std::optional<invoker_function_t> set_redirection(const std::string& from, std::string to) {
        return set(from, [to = std::move(to)](request_pointer_t request, response_pointer_t response) {
            response->set(beast::http::field::location, to);
            response_manager::edit_response(response, "text/plain", "Redirectiong to " + to, beast::http::status::moved_permanently);
        });
    }

    bool set_directory(const std::string& target, const std::string& directory_name) {
        std::string normalized_target = target;
        if (normalized_target.empty() || normalized_target.back() != '/') {
            normalized_target += '/';
        }

        std::filesystem::path dir_path = std::filesystem::absolute(directory_name);

        if (!std::filesystem::exists(dir_path) || !std::filesystem::is_directory(dir_path)) {
            spdlog::error("Directory does not exist: {}", dir_path.string());
            return false;
        }

        auto handler = [dir_path = std::move(dir_path), normalized_target]
                (request_pointer_t request, response_pointer_t response) {
            std::string url_path = request->target();

            spdlog::info(url_path);
            
            if (url_path.find(normalized_target) == std::string::npos) {
                response_manager::edit_response(response, "text/plain", "Invalid path", beast::http::status::bad_request);
                return;
            }

            std::string relative_path = url_path.substr(normalized_target.size());
            std::filesystem::path file_path = dir_path / relative_path;

            if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
                response_manager::edit_response(response, "text/plain", "File not found", beast::http::status::not_found);
                return;
            }

            file_path = std::filesystem::canonical(file_path);

            std::string content_type = response_manager::get_mime_type(file_path.extension().string());

            std::string body;
            if (auto body_ex = utils::read_file(file_path); body_ex.has_value()) {
                body = body_ex.value();
            }
            else {
                response_manager::edit_response(response, "text/plain", body_ex.error(), beast::http::status::internal_server_error);
                return;
            }

            response_manager::edit_response(response, content_type, body, beast::http::status::ok);
        };

        set(normalized_target + "*", handler);
        return true;
    }

private:
    http_server_t server;

    std::unordered_map<std::string, invoker_function_t> routes;
    invoker_function_t no_route_invoker;

    // invoker_t
    void invoker(request_pointer_t request, response_pointer_t response) {
        auto invoker_opt = get(request->target());
        (invoker_opt.has_value() ? invoker_opt.value() : no_route_invoker)(request, response);
    }

    bool target_equals(const std::string& target, const std::string& request_target) const {
        if (target.empty()) {
            return false;
        }
        if (target.back() == '*') {
            std::string normal_target(target.begin(), target.end() - 1);
            if (normal_target.empty()) {
                return true; // "*" matches everything (if intended)
            }
            return request_target.rfind(normal_target, 0) == 0; // prefix check
        }
        else {
            return target == request_target;
        }
    }
};
