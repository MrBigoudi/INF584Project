cmake_minimum_required(VERSION 3.0)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(vulkanTest)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC
                           "${PROJECT_BINARY_DIR}"
                          )

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan)

add_custom_target(
    compile_shaders
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compileShaders.sh
    COMMENT "Compiling shaders..."
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

add_dependencies(${PROJECT_NAME} compile_shaders)
