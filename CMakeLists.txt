cmake_minimum_required(VERSION 3.20)

project(SecureUSB)

set(CMAKE_CXX_STANDARD 23)

set(OPENSSL_ROOT_DIR /opt/homebrew/opt/openssl@3)

find_package(OpenSSL REQUIRED)

message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")

add_executable(SecureUSB
    main.cpp
)

target_link_libraries(SecureUSB
    OpenSSL::SSL
    OpenSSL::Crypto
)
