#pragma once

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <spdlog/spdlog.h>

struct jwt_manager {
    inline static boost::uuids::random_generator uuid_generator;

    static auto create_token(
        const std::string& username,
        const std::string& server_name,
        const std::string& secret
    ) noexcept {
        boost::uuids::uuid unique_id = uuid_generator();
        std::string jti = boost::uuids::to_string(unique_id);

        auto token = jwt::create()
            .set_issuer(server_name)
            .set_type("JWT")
            .set_id(jti)
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours{24})
            .set_payload_claim("username", jwt::claim(username))
            .sign(jwt::algorithm::hs256{secret});
        
        return token;
    }

    static bool verify_token(
        const std::string& token,
        const std::string& server_name,
        const std::string& secret
    ) noexcept try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret})
            .with_issuer(server_name);

        verifier.verify(decoded);

        spdlog::info("Token from user {} have been successly verifed!", decoded.get_payload_claim("username").as_string());
            
        return true;
    }
    catch (const jwt::error::token_verification_exception& e) {
        spdlog::info("Token verification failed: {}!", e.what());
        return false;
    }
    catch (const std::exception& e) {
        spdlog::info("JWT error: {}!", e.what());
        return false;
    }
};
