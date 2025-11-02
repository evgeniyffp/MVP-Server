#pragma once

#include <utility>

#include <nlohmann/json.hpp>

struct json_manager {
    using json = nlohmann::json;

    template <typename... Strings> 
    static bool contains(const nlohmann::json& json, Strings&&... keys) {
        return (json.contains(std::forward<Strings>(keys)) && ...);
    }

    template <typename... Strings> 
    static std::pair<json, bool> create(const std::string& json_data, Strings&&... keys) {
        if (!json::accept(json_data)) {
            return { {}, false };
        }
        auto data = json::parse(json_data);
        return { data, contains(data, std::forward<Strings>(keys)...) };
    }
};
