cmake_minimum_required(VERSION 3.15)
project(app)

set(FINAL_RELEASE FALSE)

if (FINAL_RELEASE)
	add_definitions(-DFINAL_RELEASE)
endif()

if (EMSCRIPTEN) 
	set(CMAKE_CXX_FLAGS "-O3")
	set_property(GLOBAL PROPERTY COMPILE_FLAGS "-O3")
	set_property(GLOBAL PROPERTY LINK_FLAGS "-O3")
endif()

set(CODEGEN_TOOL_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(GENERATED_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(EXECUTABLE_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(engine)
# add_subdirectory(visualization)
add_subdirectory(game)
add_subdirectory(embedTool)

set(BUILD_APPLICATIONS OFF)
add_subdirectory("dependencies/qhull")

# set(FASTNOISE2_TESTS OFF CACHE BOOL "Build Noise Tests" FORCE)
# set(FASTNOISE2_NOISETOOL OFF CACHE BOOL "Build Noise Tool" FORCE) # don't build graph tool
# add_subdirectory("dependencies/FastNoise2")