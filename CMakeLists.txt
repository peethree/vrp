cmake_minimum_required(VERSION 3.30)

# Integrate vcpkg
set(CMAKE_TOOLCHAIN_FILE "/home/pee3/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Set the project name
project(vpr)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find the cpr package
find_package(cpr REQUIRED)
find_package(nlohmann_json REQUIRED)

set(CMAKE_PREFIX_PATH "$ENV{HOME}/or-tools" ${CMAKE_PREFIX_PATH})
# set(ORTOOLS_DIR "$ENV{HOME}/or-tools")
find_package(ortools REQUIRED CONFIG)

# Add the executable
add_executable(main main.cpp)

# Link libraries
target_link_libraries(main PRIVATE cpr::cpr nlohmann_json::nlohmann_json ortools::ortools)