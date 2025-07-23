cmake_minimum_required(VERSION 3.25)

project(
    Stapel
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/Sources")
set(BIN "${CMAKE_CURRENT_BINARY_DIR}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_library(stapel_engine SHARED)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(stapel_engine PRIVATE TARGET_LINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(stapel_engine PRIVATE TARGET_WINDOWSD)
else()
    message(SEND_FATAL "Unsupported operating system ${CMAKE_SYSTEM_NAME}")
endif()

configure_file(
    "${SRC}/version.h.in"
    "${BIN}/generated/version.h"
    @ONLY
)

target_include_directories(stapel_engine PRIVATE ${BIN}/generated)
set_target_properties(stapel_engine PROPERTIES VERSION ${PROJECT_VERSION})

target_sources(
    stapel_engine
    PUBLIC FILE_SET pheaders
        TYPE HEADERS
        BASE_DIRS "${PROJECT_SOURCE_DIR}/Include"
        FILES
            "Include/Stapel/Stapel.h"
            "Include/Stapel/Application.h"
            "Include/Stapel/Renderer.h"
            "Include/Stapel/Window.h"
)

add_subdirectory(${SRC})
