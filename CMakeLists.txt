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

add_library(stb_image INTERFACE)
target_include_directories(stb_image SYSTEM INTERFACE dependencies/stb_loader)

FetchContent_Declare(glm 
		GIT_REPOSITORY https://github.com/g-truc/glm.git
		GIT_TAG master 
)
FetchContent_MakeAvailable(glm)
target_include_directories(glm SYSTEM INTERFACE ${glm_SOURCE_DIR})

FetchContent_Declare(entt
		GIT_REPOSITORY 	https://github.com/skypjack/entt.git
		GIT_TAG master 
)
FetchContent_MakeAvailable(entt)
add_library(entt INTERFACE) 
target_include_directories(entt SYSTEM INTERFACE ${entt_SOURCE_DIR}/src)

FetchContent_Declare(imgui
		GIT_REPOSITORY https://github.com/ocornut/imgui.git
		GIT_TAG master
)
FetchContent_MakeAvailable(imgui)
add_library(imgui STATIC
		${imgui_SOURCE_DIR}/imgui.cpp
		${imgui_SOURCE_DIR}/imgui_demo.cpp
		${imgui_SOURCE_DIR}/imgui_draw.cpp
		${imgui_SOURCE_DIR}/imgui_tables.cpp
		${imgui_SOURCE_DIR}/imgui_widgets.cpp
		${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
		${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC 
		${imgui_SOURCE_DIR}
		${imgui_SOURCE_DIR}/backends
)

FetchContent_Declare(assimp  
    GIT_REPOSITORY https://github.com/assimp/assimp.git 
    GIT_TAG master
) 
set(ASSIMP_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

get_target_property(assimp_includes assimp INTERFACE_INCLUDE_DIRECTORIES)
if(assimp_includes)
    set_target_properties(assimp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "")
    target_include_directories(assimp SYSTEM INTERFACE ${assimp_includes})
endif()

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp src/shader.cpp src/mesh.cpp src/model.cpp src/stb_image_loader.cpp src/app.cpp)
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
-Wloop-analysis
-Wmove
-Wcomma
-Wdocumentation
)
target_link_libraries(${PROJECT_NAME} PRIVATE fontconfig glfw glad glm stb_image assimp imgui entt)

message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "This project requires Clang compiler")
endif()


