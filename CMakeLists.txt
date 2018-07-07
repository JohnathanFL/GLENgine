cmake_minimum_required(VERSION 3.7)
project(GLENgine)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(deps)
add_subdirectory(glengine)


add_executable(GLENgine_test "test/main.cpp")
target_include_directories(GLENgine_test PRIVATE "glengine")
target_link_libraries(GLENgine_test GLENgine)
