cmake_minimum_required(VERSION 3.15)

set(FINAL_RELEASE FALSE)

if (FINAL_RELEASE)
	add_definitions(-DFINAL_RELEASE)
endif()


set(CODEGEN_TOOL_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(GENERATED_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(EXECUTABLE_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(engine)
# add_subdirectory(visualization)
add_subdirectory(game)

set(BUILD_APPLICATIONS OFF)
add_subdirectory("dependencies/qhull")
