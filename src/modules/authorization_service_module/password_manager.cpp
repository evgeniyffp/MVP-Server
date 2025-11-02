#include "./password_manager.h"

#include <bcrypt/BCrypt.hpp>

namespace password_manager {
    std::string hash(const std::string& password) noexcept {
        return BCrypt::generateHash(password);
    }

    bool verify(const std::string& password, const std::string& password_hash) noexcept {
        return BCrypt::validatePassword(password, password_hash);
    }

    complexity_e check_complexity(const std::string& password) noexcept {
        int lowercase_letters = 0,
            uppercase_letters = 0,
            digits = 0,
            underscores = 0;
    
        for (auto c : password) {
            if (c >= 'a' && c <= 'z')
                ++lowercase_letters;
            else if (c >= 'A' && c <= 'Z')
                ++uppercase_letters;
            else if (c >= '0' && c <= '9')
                ++digits;
            else if (c == '_')
                ++underscores;
            else
                return complexity_e::invalid;
        }

        int lenght = password.size();

        if (lenght >= 12 && lowercase_letters >= 2 && uppercase_letters >= 2 && digits >= 2 && underscores >= 1)
            return complexity_e::hard;
        else if (lenght >= 8 && lowercase_letters >= 1 && uppercase_letters >= 1 && digits >= 1)
            return complexity_e::medium;
        else
            return complexity_e::easy;
    }
}