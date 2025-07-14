cmake_minimum_required(VERSION 3.20.0)
project(Engine VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)

add_library(glad STATIC )
target_sources(glad PRIVATE dependencies/glad/src/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
target_include_directories(glad PUBLIC dependencies/glad/include)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE dependencies/glm)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp src/shader.cpp)
add_compile_options(
-Wall
-Wextra
-Wpedantic
-Werror
-Wconversion
-Wsign-conversion
-Wshadow
-Wnon-virtual-dtor
-Wold-style-cast
-Wcast-align
-Wunreachable-code
-Wunused
-Wunused-parameter
-Wstrict-null-sentinel
-Wlogical-op
-fsanitize=address
)
add_link_options(-fsanitize=address)

target_link_libraries(${PROJECT_NAME} PRIVATE fontconfig glfw glad glm)
target_include_directories(${PROJECT_NAME} PRIVATE src  dependencies/stb_loader)
