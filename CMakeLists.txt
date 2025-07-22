cmake_minimum_required(VERSION 3.25)

project(
    Stapel
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_library(Stapel SHARED)

if(UNIX AND NOT MACOS)
    target_compile_definitions(Stapel PRIVATE TARGET_LINUX)
elseif(WIN32)
    target_compile_definitions(Stapel PRIVATE TARGET_WINDOWSD)
endif()

target_sources(
    Stapel
    PUBLIC FILE_SET pheaders
        TYPE HEADERS
        BASE_DIRS "${PROJECT_SOURCE_DIR}/Include"
        FILES
            "Include/Stapel/Stapel.h"
)

add_subdirectory(${PROJECT_SOURCE_DIR}/Sources)
