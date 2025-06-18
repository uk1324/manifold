add_executable(game "main.cpp" "MainLoop.cpp"  "GameRenderer.cpp" "Tri3d.cpp" "MeshUtils.cpp" "FpsCamera3d.cpp" "Constants.cpp" "Polyhedra.cpp" "DoublyConnectedEdgeList.cpp"  "PerlinNoise.cpp" "Permutations.cpp" "Stereographic.cpp" "LineGenerator.cpp" "Bezier.cpp" "Game.cpp" "Polytopes.cpp" "Combinatorics.cpp" "StereographicCamera.cpp" "Math.cpp" "Physics/World.cpp"  "Physics/Body.cpp" "4d.cpp" "Physics/ContactConstraint.cpp" "Physics/Collide.cpp" "ConvexHull.cpp" "Noise.cpp" "Minesweeper.cpp" "Tiling.cpp" "WindowUtils.cpp" "Animation.cpp")

if (EMSCRIPTEN)
	set_target_properties(game PROPERTIES OUTPUT_NAME "index")
	set_target_properties(game PROPERTIES SUFFIX ".html")

	# set_target_properties(game PROPERTIES COMPILE_FLAGS "-O3")
	# set_target_properties(game PROPERTIES LINK_FLAGS "-O3")

	target_compile_options(game PUBLIC "--use-port=contrib.glfw3")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --use-port=contrib.glfw3")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASSERTIONS=1 -s WASM=1 -s SAFE_HEAP=1 -s MAX_WEBGL_VERSION=2")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s FULL_ES3 --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html")

	target_link_libraries(game PUBLIC glfw3)
endif()

target_link_libraries(game PUBLIC engine)

target_compile_features(game PUBLIC cxx_std_23)
set_target_properties(game PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(game PUBLIC "../" "../engine/dependencies/")

include("../engine/codeGenTool/targetAddGenerated.cmake")

target_link_libraries(game PUBLIC gfx2d)

target_include_directories(game PUBLIC "../dependencies/qhull/src/")
target_link_libraries(game PUBLIC qhullcpp)

# target_include_directories(game PUBLIC "../dependencies/FastNoise2/include/")
# target_link_libraries(game PUBLIC FastNoise2)

if (NOT EMSCRIPTEN)
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/../engine/dependencies/freetype.dll" 
		"${CMAKE_CURRENT_SOURCE_DIR}/../freetype.dll" 
		COPYONLY)
endif()

if (MSVC)
	if (WIN32)
		target_link_libraries(game PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../engine/dependencies/freetype.lib")
	else()
		target_link_libraries(game PUBLIC freetypeStaticRelease)
	endif()
endif()

if (MSVC)
	# enumerator 'identifier' in switch of enum 'enumeration' is not handled
	target_compile_options(game PRIVATE /we4062)
endif()

targetAddGenerated(game ${CMAKE_CURRENT_SOURCE_DIR})

# If this is on, then the console logs won't show even when launched from console.
if (FINAL_RELEASE)
	if (WIN32)
		set_target_properties(game PROPERTIES WIN32_EXECUTABLE TRUE)
	endif()
endif()