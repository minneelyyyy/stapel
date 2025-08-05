
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(stapel_launcher)
    add_subdirectory(Linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_executable(stapel_launcher)
    set_target_properties(stapel_launcher PROPERTIES LINK_FLAGS "/ENTRY:WinMainCRTStartup")
    target_include_directories(stapel_launcher PRIVATE "${CMAKE_SOURCE_DIR}/Include")
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
