

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

add_custom_command(
    OUTPUT ${BIN}/shaders/gradient.comp.spv
    COMMAND ${GLSL_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/gradient.comp -o ${BIN}/shaders/gradient.comp.spv
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/gradient.comp
)

add_custom_target(
    shaders
    DEPENDS
        ${BIN}/shaders/gradient.comp.spv
)

add_dependencies(
    StapelEngine
    shaders
)

target_sources(
    StapelEngine
    PRIVATE
        VulkanRenderer.cpp
        VulkanRenderer.h
        DeletionQueue.cpp
        DeletionQueue.h
        Device.cpp
        Device.h
        Swapchain.cpp
        Swapchain.h
        CommandPool.cpp
        CommandPool.h
        CommandBuffer.cpp
        CommandBuffer.h
        Pipeline.cpp
        Pipeline.h
        Image.cpp
        Image.h
        util.cpp
        util.h
)
