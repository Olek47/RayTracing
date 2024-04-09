cmake_minimum_required(VERSION 3.14)

project(RayTracing VERSION 1.0)

# Default to Release build mode
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add sources
add_executable(RayTracing src/Main.cpp src/Application.cpp src/Image.cpp src/Random.cpp src/Scene.cpp)

# Set compiler flags
if(MSVC)
    # TODO
else()
    target_compile_options(RayTracing PRIVATE -Wall -Wextra -Wno-missing-field-initializers)
endif()

# GLM
find_package(glm REQUIRED)
target_link_libraries(RayTracing glm::glm)

# JSON for Modern C++
find_package(nlohmann_json REQUIRED)
target_link_libraries(RayTracing nlohmann_json::nlohmann_json)

# Add external header only libraries
target_include_directories(RayTracing PRIVATE include)