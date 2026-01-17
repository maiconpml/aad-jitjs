# Instance Generator

# Gather Sources (assuming there will be sources in src/)
file(GLOB_RECURSE SOURCES "src/*.cpp")
set(MAIN "main.cpp")

add_executable(instance-generator ${MAIN} ${SOURCES})

# Link Libraries (assuming it needs Boost too, or just PCG which is header only)
target_link_libraries(instance-generator ${Boost_LIBRARIES})
