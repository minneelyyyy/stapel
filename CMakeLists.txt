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

add_library(StapelEngine SHARED)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(StapelEngine PRIVATE TARGET_LINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(StapelEngine PRIVATE TARGET_WINDOWS)
    target_compile_definitions(StapelEngine PRIVATE STAPEL_BUILD_DLL)
else()
    message(SEND_FATAL "Unsupported operating system ${CMAKE_SYSTEM_NAME}")
endif()

configure_file(
    "${SRC}/version.h.in"
    "${BIN}/generated/version.h"
    @ONLY
)

target_include_directories(StapelEngine PRIVATE ${BIN}/generated)
set_target_properties(StapelEngine PROPERTIES VERSION ${PROJECT_VERSION})

FILE(GLOB_RECURSE PUBLIC_INCLUDES "${PROJECT_SOURCE_DIR}/Include/*")

target_sources(
    StapelEngine
    PUBLIC FILE_SET HEADERS
        BASE_DIRS "${PROJECT_SOURCE_DIR}/Include"
        FILES
            ${PUBLIC_INCLUDES}
)

target_include_directories(StapelEngine PRIVATE ${SRC})
add_subdirectory(${SRC})

install(
    TARGETS StapelEngine
    EXPORT StapelEngineTargets
    FILE_SET HEADERS
    RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION bin
)

install(
    EXPORT StapelEngineTargets
    FILE StapelTargets.cmake
    NAMESPACE Stapel::
    DESTINATION lib/cmake/Stapel
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/StapelConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/StapelConfig.cmake
    INSTALL_DESTINATION lib/cmake/Stapel
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/StapelConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/StapelConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/StapelConfigVersion.cmake
    DESTINATION lib/cmake/Stapel
)

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/NOTICE"
    DESTINATION notices
    RENAME STAPEL-NOTICE.txt
)
