cmake_minimum_required(VERSION 3.22)
project(dotchat_client)

set(CONAN_EXIT_CODE not_run_yet)
message("Trying to run `conan install ..` from ${CMAKE_BINARY_DIR}")
EXECUTE_PROCESS(
        COMMAND "conan" "install" ".." "--build=missing"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        RESULT_VARIABLE CONAN_EXIT_CODE
)
message("Result: ${CONAN_EXIT_CODE}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(${PROJECT_NAME} main.cpp
        ../shared/src/tls/tls_client_socket.cpp ../shared/src/tls/tls_context.cpp ../shared/src/tls/tls_connection.cpp
        ../shared/src/protocol/message.cpp ../shared/src/protocol/message_intl.cpp
        ../shared/src/protocol/requests.cpp ../shared/src/protocol/responses.cpp
        main.cpp
        src/cli/wait_loop.cpp src/cli/login_related.cpp src/cli/channel_related.cpp src/cli/user_related.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE inc/)
target_include_directories(${PROJECT_NAME} PRIVATE ../shared/inc/)
conan_target_link_libraries(${PROJECT_NAME})