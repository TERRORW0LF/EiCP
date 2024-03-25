# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.12)

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
  XPBDCloth
  VERSION 1.0)

# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# Find packages go here.
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Adding something we can run - Output name matches target name
add_executable(XPBDCloth
        src/config.h
        src/main.cpp
        src/glad.c
        src/cloth_mesh.h
        src/cloth_mesh.cpp
        src/physics_engine.h
        src/physics_engine.cpp
        src/linear_algebra.h
        src/linear_algebra.cpp
	src/algebraic_types.h
        src/algebraic_types.cpp
        src/spatial_hash_structure.h
        src/spatial_hash_structure.cpp
        src/xpbd_window.h
        src/xpbd_window.cpp
        src/shader.h
        src/shader.cpp
        src/camera.h
        src/camera.cpp
        src/obj_reader.h
        src/obj_reader.cpp
)

target_include_directories(XPBDCloth PRIVATE dependencies C:/msys64/mingw64/include)

target_link_libraries(XPBDCloth PRIVATE glfw OpenGL::GL)

add_custom_command(TARGET ${PROJECT_NAME}  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/glfw3.dll
                ${CMAKE_CURRENT_BINARY_DIR}/glfw3.dll
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/assets
                ${CMAKE_CURRENT_BINARY_DIR}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders
                ${CMAKE_CURRENT_BINARY_DIR}/shaders)