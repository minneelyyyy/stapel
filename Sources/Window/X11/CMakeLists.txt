target_sources(
    Stapel
    PRIVATE
        X11Window.cpp
    PRIVATE FILE_SET HEADERS
        BASE_DIRS "${CMAKE_SOURCE_DIR}/Sources"
        FILES
            X11Window.h
)