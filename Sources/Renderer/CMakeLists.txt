
find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    add_subdirectory(Vulkan)
    target_compile_definitions(stapel_engine PRIVATE VULKAN_ENABLED)
    target_include_directories(stapel_engine PRIVATE ${Vulkan_INCLUDE_DIR})
    target_link_libraries(stapel_engine PRIVATE ${Vulkan_LIBRARIES})
else()
    message(FATAL_ERROR "Vulkan SDK not found on the system.")
endif()

target_sources(
    stapel_engine
    PRIVATE
        Renderer.cpp
    PRIVATE FILE_SET HEADERS
        BASE_DIRS "${CMAKE_SOURCE_DIR}/Sources"
        FILES
            Renderer.h
)
