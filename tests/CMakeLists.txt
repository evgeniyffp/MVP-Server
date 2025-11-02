include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(PROJECT_TESTS_NAME ${CMAKE_PROJECT_NAME}_tests)
set(PROJECT_TESTS_SOURCE main.cpp)

add_executable(${PROJECT_TESTS_NAME} ${PROJECT_TESTS_SOURCE})

target_link_libraries(${PROJECT_TESTS_NAME} PRIVATE sBeHappy_library GTest::gtest_main)

add_test(NAME ${PROJECT_TESTS_NAME} COMMAND ${PROJECT_TESTS_NAME})