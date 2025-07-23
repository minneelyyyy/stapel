target_sources(
    stapel_engine
    PRIVATE
        WlWindow.cpp
    PRIVATE FILE_SET HEADERS
        BASE_DIRS "${CMAKE_SOURCE_DIR}/Sources"
        FILES
            WlWindow.h
)