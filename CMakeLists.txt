cmake_minimum_required(VERSION 3.30)

# Integrate vcpkg
set(CMAKE_TOOLCHAIN_FILE "/home/pee3/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Set the project name
project(vpr)

# Set the C++ standard 
set(CMAKE_CXX_STANDARD 17) # ortools and cpr require at least 17 I believe
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES
    src/main.cpp
    src/assignment.cpp
)

# Find the cpr package
find_package(cpr REQUIRED)
find_package(nlohmann_json REQUIRED)

# Prefix path for ortools so "find_package" can reach it
set(CMAKE_PREFIX_PATH "$ENV{HOME}/or-tools" ${CMAKE_PREFIX_PATH})
find_package(ortools REQUIRED CONFIG)

# Add the executable
add_executable(main ${SOURCES})

# Link libraries
target_link_libraries(main PRIVATE cpr::cpr nlohmann_json::nlohmann_json ortools::ortools)