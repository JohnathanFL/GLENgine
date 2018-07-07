cmake_minimum_required(VERSION 3.0)


file(GLOB_RECURSE GLENGINE_SRCS "./*.*pp")

add_library(${PROJECT_NAME} ${GLENGINE_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC $ENV{VULKAN_SDK}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ../deps/enummap)
target_include_directories(${PROJECT_NAME} PUBLIC ../deps/entt/src)


target_link_libraries(${PROJECT_NAME} vulkan dl SDL2)
