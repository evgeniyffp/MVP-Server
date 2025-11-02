#pragma once

#include <string>

namespace password_manager {
    enum class complexity_e {
        invalid, easy, medium, hard
    };

    std::string hash(const std::string& password) noexcept;

    bool verify(const std::string& password, const std::string& password_hash) noexcept;

    complexity_e check_complexity(const std::string& password) noexcept;
}
