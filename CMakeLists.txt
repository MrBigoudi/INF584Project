cmake_minimum_required(VERSION 3.0)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# c++ standard
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(cflags ${PROJECT_NAME}_compiler_flags)
add_library(cflags INTERFACE)
target_compile_features(cflags INTERFACE cxx_std_20)
target_compile_options(cflags INTERFACE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

project(vulkanTest)

add_executable(${PROJECT_NAME} main.cpp)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory(src)
add_subdirectory(dep)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan cflags)
target_link_libraries(${PROJECT_NAME} PRIVATE BigoudiEngine cflags)