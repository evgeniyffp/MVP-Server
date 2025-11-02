set(PROJECT_SOURCES
    https_server_module/https_server.cpp
    authorization_service_module/authorization_service.cpp
    authorization_service_module/password_manager.cpp
    utils_module/utils.cpp
)

set(PROJECT_LIBRARY_NAME ${CMAKE_PROJECT_NAME}_library)

add_library(${PROJECT_LIBRARY_NAME} STATIC ${PROJECT_SOURCES})

target_include_directories(${PROJECT_LIBRARY_NAME} PUBLIC ../include/)
target_link_libraries(${PROJECT_LIBRARY_NAME}
    INTERFACE
    pqxx pq
    Boost::system
    OpenSSL::SSL
    bcrypt
    spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>
    fmt::fmt
#     jwt-cpp::jwt-cpp
)
