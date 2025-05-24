add_executable(game "main.cpp" "MainLoop.cpp"  "GameRenderer.cpp" "Tri3d.cpp" "MeshUtils.cpp" "FpsCamera3d.cpp" "Constants.cpp" "Polyhedra.cpp" "DoublyConnectedEdgeList.cpp"  "PerlinNoise.cpp" "Permutations.cpp" "Stereographic.cpp" "LineGenerator.cpp" "Bezier.cpp" "Game.cpp" "Polytopes.cpp" "Combinatorics.cpp" "StereographicCamera.cpp" "Math.cpp" "Physics/World.cpp"  "Physics/Body.cpp" "4d.cpp" "Physics/ContactConstraint.cpp" "Physics/Collide.cpp")

target_link_libraries(game PUBLIC engine)

target_compile_features(game PUBLIC cxx_std_23)
set_target_properties(game PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(game PUBLIC "../" "../engine/dependencies/")

include("../engine/codeGenTool/targetAddGenerated.cmake")

target_link_libraries(game PUBLIC gfx2d)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/../engine/dependencies/freetype.dll" 
	"${CMAKE_CURRENT_SOURCE_DIR}/../freetype.dll" 
	COPYONLY)

if (WIN32)
	target_link_libraries(game PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../engine/dependencies/freetype.lib")
else()
	target_link_libraries(game PUBLIC freetypeStaticRelease)
endif()


# enumerator 'identifier' in switch of enum 'enumeration' is not handled
target_compile_options(game PRIVATE /we4062)

targetAddGenerated(game ${CMAKE_CURRENT_SOURCE_DIR})

# If this is on, then the console logs won't show even when launched from console.
if (FINAL_RELEASE)
	if (WIN32)
		set_target_properties(game PROPERTIES WIN32_EXECUTABLE TRUE)
	endif()
endif()