
add_executable(stapel_launcher)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(
        stapel_launcher
        PRIVATE
            Linux/main.cpp
    )

    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    set_target_properties(
        stapel_launcher
        PROPERTIES
            BUILD_RPATH "$ORIGIN/stapel"
    )

    target_sources(
        stapel_engine
        PRIVATE
            Linux/GetApplication.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(
        stapel_launcher
        PRIVATE
            Windows/winmain.cpp
    )
endif()

target_sources(
    stapel_engine
    PRIVATE
        Entry.cpp
)
