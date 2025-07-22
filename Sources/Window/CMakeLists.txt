cmake_minimum_required(VERSION 3.25)

if(UNIX AND NOT MACOS)
    find_package(X11)
    find_package(Wayland)

    if(Wayland_FOUND)
        add_subdirectory(Wayland)
        target_compile_definitions(Stapel PRIVATE WAYLAND_ENABLED)
        target_include_directories(Stapel PRIVATE ${Wayland_INCLUDE_DIR})
        target_link_libraries(Stapel PRIVATE ${Wayland_LIBRARIES})
    endif()
    if(X11_FOUND)
        add_subdirectory(X11)
        target_compile_definitions(Stapel PRIVATE X11_ENABLED)
        target_include_directories(Stapel PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(Stapel PRIVATE ${X11_LIBRARIES})
    endif()
endif()

target_sources(
    Stapel
    PRIVATE
        Window.cpp
    PRIVATE FILE_SET HEADERS
        BASE_DIRS "${CMAKE_SOURCE_DIR}/Sources"
        FILES
            Window.h
)
