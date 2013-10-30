cmake_minimum_required(VERSION 2.6)
project(qimaptray)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Werror")


find_package(OpenSSL REQUIRED)

add_executable(qimaptray
  imap.cpp
  status.cpp
  qimaptray.cpp
)

target_link_libraries(qimaptray ssl)
