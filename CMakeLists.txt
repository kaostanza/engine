cmake_minimum_required(VERSION 3.20.0)
project(Engine VERSION 1.0.0 LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)

add_library(glad STATIC)
target_sources(glad PRIVATE dependencies/glad/src/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
target_include_directories(glad PUBLIC dependencies/glad/include)

add_library(glm INTERFACE)
target_include_directories(glm SYSTEM INTERFACE dependencies/glm)

add_library(stb_image INTERFACE)
target_include_directories(stb_image SYSTEM INTERFACE dependencies/stb_loader)

FetchContent_Declare(assimp  GIT_REPOSITORY https://github.com/assimp/assimp.git GIT_TAG master) 
set(ASSIMP_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp src/shader.cpp src/mesh.cpp src/model.cpp src/stb_image_loader.cpp)
target_compile_options(${PROJECT_NAME} PRIVATE
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
target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
target_link_libraries(${PROJECT_NAME} PRIVATE fontconfig glfw glad glm stb_image assimp)


