#pragma once

namespace role_manager {
    enum class role_e {
        invalid, buyer, seller, root, admin
    };

    role_e check(const std::string& role) {
        if (role == "buyer") return role_e::buyer;
        if (role == "seller") return role_e::seller;
        if (role == "root") return role_e::root;
        if (role == "admin") return role_e::admin;

        return role_e::invalid;
    }
};