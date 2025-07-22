
find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    add_subdirectory(Vulkan)
    target_compile_definitions(Stapel PRIVATE VULKAN_ENABLED)
    target_include_directories(Stapel PRIVATE ${Vulkan_INCLUDE_DIR})
    target_link_libraries(Stapel PRIVATE ${Vulkan_LIBRARIES})
else()
    message(FATAL_ERROR "Vulkan SDK not found on your system.")
endif()

target_sources(
    Stapel
    PRIVATE
        Renderer.cpp
    PRIVATE FILE_SET HEADERS
        BASE_DIRS "${CMAKE_SOURCE_DIR}/Sources"
        FILES
            Renderer.h
)
