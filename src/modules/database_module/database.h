#pragma once

#include <pqxx/pqxx>

#include <string>

class pqxx_wrapper {
private:
    pqxx::connection connection;

public:
    pqxx_wrapper(const std::string& setting) : connection(setting) {}

    pqxx_wrapper(const pqxx_wrapper&) = delete;
    pqxx_wrapper& operator=(const pqxx_wrapper&) = delete;

    pqxx::work get_transaction() {
        return pqxx::work(connection); 
    }
};

using database = pqxx_wrapper;
