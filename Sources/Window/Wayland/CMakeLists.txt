find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
find_path(WAYLAND_PROTOCOLS_DIR
    NAMES stable/xdg-shell/xdg-shell.xml
    HINTS /usr/share/wayland-protocols
    PATH_SUFFIXES wayland-protocols
    REQUIRED
)

set(XDG_SHELL_XML ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml)
set(XDG_WDM_XML ${WAYLAND_PROTOCOLS_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml)

set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GEN_DIR})

set(GENERATED_XDG_C ${GEN_DIR}/xdg-shell-protocol.c)
set(GENERATED_XDG_H ${GEN_DIR}/xdg-shell-client-protocol.h)

set(GENERATED_WDM_C ${GEN_DIR}/xdg-decoration-unstable-v1.c)
set(GENERATED_WDM_H ${GEN_DIR}/xdg-decoration-unstable-v1-protocol.h)

add_custom_command(
    OUTPUT
        ${GENERATED_XDG_C}
        ${GENERATED_XDG_H}
        ${GENERATED_WDM_C}
        ${GENERATED_WDM_H}
    COMMAND ${WAYLAND_SCANNER} private-code ${XDG_SHELL_XML} ${GENERATED_XDG_C}
    COMMAND ${WAYLAND_SCANNER} client-header ${XDG_SHELL_XML} ${GENERATED_XDG_H}
    COMMAND ${WAYLAND_SCANNER} private-code ${XDG_WDM_XML} ${GENERATED_WDM_C}
    COMMAND ${WAYLAND_SCANNER} client-header ${XDG_WDM_XML} ${GENERATED_WDM_H}
    DEPENDS
        ${XDG_SHELL_XML}
        ${XDG_WDM_XML}
    VERBATIM
)

add_custom_target(generate_xdg_shell ALL
    DEPENDS ${GENERATED_XDG_C} ${GENERATED_XDG_H} ${GENERATED_WDM_C} ${GENERATED_WDM_H}
)

add_dependencies(StapelEngine generate_xdg_shell)

target_sources(StapelEngine PRIVATE
    ${GENERATED_XDG_C}
    ${GENERATED_XDG_H}
    ${GENERATED_WDM_C}
    ${GENERATED_WDM_H}
    WlWindow.cpp
    WlWindow.h
)

target_include_directories(StapelEngine PRIVATE ${GEN_DIR})
target_include_directories(StapelEngine PRIVATE ${Wayland_INCLUDE_DIR})
target_link_libraries(StapelEngine PRIVATE ${Wayland_LIBRARIES})
