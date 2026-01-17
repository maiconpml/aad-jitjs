# Find CPLEX
find_package(CPLEX REQUIRED)

# Include directories for Solver
include_directories(src)
include_directories(${CPLEX_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

# Gather Sources
file(GLOB_RECURSE SOURCES "src/*.cpp")
set(MAIN "main.cpp")

add_executable(solver ${MAIN} ${SOURCES})

# Link Libraries
target_link_libraries(solver ${Boost_LIBRARIES} ${CPLEX_LIBRARIES})
