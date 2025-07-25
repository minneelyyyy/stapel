
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(stapel_launcher)
    add_subdirectory(Linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_executable(stapel_launcher WIN32)
    add_subdirectory(Windows)
endif()

install(
    TARGETS stapel_launcher
    DESTINATION bin
)

include(StapelDist)
stapel_dist_add_target(
    TARGET stapel_launcher
    INSTALL_DESTINATION bin
    DIST_DESTINATION .
)

target_sources(
    StapelEngine
    PRIVATE
        Entry.cpp
)
