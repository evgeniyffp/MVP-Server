add_subdirectory(modules)

add_executable(${CMAKE_PROJECT_NAME} app/main.cpp app/application.cpp)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC include/)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_PROJECT_NAME}_library)
