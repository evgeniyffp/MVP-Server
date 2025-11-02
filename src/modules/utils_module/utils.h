#pragma once

#include <string>
#include <expected>
#include <fstream>

namespace utils {
    template <class T>
    struct config_t;

    std::expected<std::string, std::string> read_file(const std::string& file_name);
}

using utils::config_t;
