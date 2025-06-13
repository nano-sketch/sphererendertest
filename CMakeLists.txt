cmake_minimum_required(VERSION 3.10)
project(SphereRenderer)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Find ImGui source directory
find_path(IMGUI_SOURCE_DIR imgui.cpp
    PATHS 
    "C:/Users/nxr/Documents/vcpkg/buildtrees/imgui/src"
    "C:/Users/nxr/Documents/vcpkg/downloads/imgui"
    PATH_SUFFIXES "v1.91.8-164364f825.clean"
)

# Set ImGui backend headers path
set(IMGUI_BACKEND_HEADERS_DIR "C:/Users/nxr/Documents/vcpkg/installed/x64-windows/include/imgui/backends")
set(IMGUI_INTERNAL_HEADERS_DIR "C:/Users/nxr/Documents/vcpkg/installed/x64-windows/include/imgui")

# Print out paths for debugging
message(STATUS "IMGUI_SOURCE_DIR: ${IMGUI_SOURCE_DIR}")
message(STATUS "IMGUI_BACKEND_HEADERS_DIR: ${IMGUI_BACKEND_HEADERS_DIR}")
message(STATUS "IMGUI_INTERNAL_HEADERS_DIR: ${IMGUI_INTERNAL_HEADERS_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")

# Ensure backend headers are copied to the source directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/backends)

# List of headers to copy
set(IMGUI_HEADERS_TO_COPY
    "${IMGUI_SOURCE_DIR}/imgui.h"
    "${IMGUI_SOURCE_DIR}/imgui_internal.h"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.h"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.h"
)

# Copy headers if they exist
foreach(HEADER ${IMGUI_HEADERS_TO_COPY})
    get_filename_component(HEADER_NAME "${HEADER}" NAME)
    if(EXISTS "${HEADER}")
        file(COPY "${HEADER}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/src/backends")
        message(STATUS "Copied header: ${HEADER_NAME}")
    else()
        message(WARNING "Header not found: ${HEADER}")
    endif()
endforeach()

# Set output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)

# Add executable
add_executable(sphere_renderer 
    src/main.cpp
    ${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Ensure the output directory exists
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})

# Link libraries
target_link_libraries(sphere_renderer 
    glfw
    GLEW::GLEW
    OpenGL::GL
    imgui::imgui
    glm::glm
)

# Include directories
target_include_directories(sphere_renderer PRIVATE 
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${IMGUI_INCLUDE_DIRS}
    ${IMGUI_SOURCE_DIR}
    ${IMGUI_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backends
    "C:/Users/nxr/Documents/vcpkg/installed/x64-windows/include/imgui"
    "C:/Users/nxr/Documents/vcpkg/installed/x64-windows/include/imgui/backends"
) 