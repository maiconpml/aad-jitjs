cmake_minimum_required(VERSION 3.10)
project(AAD_JITJS VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add custom modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Include directories (Global)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find Boost (Required for both)
find_package(Boost REQUIRED COMPONENTS program_options)

# Add Subdirectories
add_subdirectory(programs/solver)
add_subdirectory(programs/instance-generator)
