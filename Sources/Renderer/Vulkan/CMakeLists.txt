target_sources(
    Stapel
    PRIVATE
        VulkanRenderer.cpp
    PRIVATE FILE_SET HEADERS
        BASE_DIRS "${CMAKE_SOURCE_DIR}/Sources"
        FILES
            VulkanRenderer.h
)
