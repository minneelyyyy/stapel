target_sources(
    stapel_engine
    PRIVATE
        WinWindow.cpp
    PRIVATE FILE_SET HEADERS
        BASE_DIRS "${CMAKE_SOURCE_DIR}/Sources"
        FILES
            WinWindow.h
)