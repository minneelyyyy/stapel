cmake_minimum_required(VERSION 3.25)

project(Example VERSION 0.0.1 LANGUAGES CXX)

if(NOT DEFINED ENV{STAPEL_SDK})
    message(FATAL_ERROR "STAPEL_SDK environment variable is not set.")
endif()

list(APPEND CMAKE_PREFIX_PATH "$ENV{STAPEL_SDK}")

add_library(game SHARED Example.cpp)

find_package(Stapel REQUIRED)

target_link_libraries(game PRIVATE Stapel::StapelEngine)
