cmake_minimum_required(VERSION 3.25)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11)
    find_package(Wayland)

    if(Wayland_FOUND)
        add_subdirectory(Wayland)
        target_compile_definitions(StapelEngine PRIVATE WAYLAND_ENABLED)
        target_include_directories(StapelEngine PRIVATE ${Wayland_INCLUDE_DIR})
        target_link_libraries(StapelEngine PRIVATE ${Wayland_LIBRARIES})
    endif()
    if(X11_FOUND)
        add_subdirectory(X11)
        target_compile_definitions(StapelEngine PRIVATE X11_ENABLED)
        target_include_directories(StapelEngine PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(StapelEngine PRIVATE ${X11_LIBRARIES})
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(Windows)
endif()

target_sources(
    StapelEngine
    PRIVATE
        Window.cpp
        Window.h
)
