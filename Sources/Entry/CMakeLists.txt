
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(stapel_launcher)

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

    install(
        TARGETS stapel_launcher
        DESTINATION bin
    )

    target_sources(
        StapelEngine
        PRIVATE
            Linux/GetApplication.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_executable(stapel_launcher WIN32)

    target_sources(
        stapel_launcher
        PRIVATE
            Windows/winmain.cpp
    )

    install(
        TARGETS stapel_launcher
        DESTINATION bin
    )

    target_sources(
        StapelEngine
        PRIVATE
            Windows/GetApplication.cpp
    )
endif()

target_sources(
    StapelEngine
    PRIVATE
        Entry.cpp
)
