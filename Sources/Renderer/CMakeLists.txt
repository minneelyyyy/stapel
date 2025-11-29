
find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    add_subdirectory(Vulkan)
    target_compile_definitions(StapelEngine PRIVATE USE_VULKAN=1)
    target_include_directories(StapelEngine PRIVATE ${Vulkan_INCLUDE_DIR})
    target_link_libraries(StapelEngine PRIVATE ${Vulkan_LIBRARIES})
else()
    message(FATAL_ERROR "Vulkan SDK not found on the system.")
endif()

target_sources(
    StapelEngine
    PRIVATE
        Renderer.cpp
        Renderer.h
)
