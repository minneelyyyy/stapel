
add_subdirectory(Window)
add_subdirectory(Renderer)

# add test program (to be removed later)
add_executable(Test main.cpp)
target_link_libraries(Test Stapel)
