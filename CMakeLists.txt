cmake_minimum_required(VERSION 3.10)
project(AAD_JITJS VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb -O0 -Wall -Wextra -pedantic -Wshadow -Wconversion -Wformat=2 -fsanitize=address,undefined -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

# Enable Link Time Optimization (LTO)
include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)
if(supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "IPO/LTO is supported and enabled")
else()
    message(STATUS "IPO/LTO is not supported: ${error}")
endif()

# Add custom modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Include directories (Global)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find Boost (Required for both)
find_package(Boost REQUIRED COMPONENTS program_options)

# Add Subdirectories
add_subdirectory(programs/solver)
add_subdirectory(programs/instance-generator)
