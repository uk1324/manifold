add_executable(game "main.cpp" "MainLoop.cpp" "Camera3d.cpp" "Renderer.cpp" "PlotUtils.cpp" "Surfaces/Torus.cpp" "Surfaces/Sphere.cpp" "Surfaces/Helicoid.cpp" "Surfaces/Cone.cpp" "Surfaces/MobiusStrip.cpp" "Surfaces/Pseudosphere.cpp" "Surfaces/Trefoil.cpp")

target_link_libraries(game PUBLIC engine)

target_compile_features(game PUBLIC cxx_std_23)
set_target_properties(game PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(game PUBLIC "../" "../engine/dependencies/")

include("../engine/codeGenTool/targetAddGenerated.cmake")

target_link_libraries(game PUBLIC gfx2d)

# enumerator 'identifier' in switch of enum 'enumeration' is not handled
target_compile_options(game PRIVATE /we4062)

targetAddGenerated(game ${CMAKE_CURRENT_SOURCE_DIR})

# If this is on, then the console logs won't show even when launched from console.
if (FINAL_RELEASE)
	if (WIN32)
		set_target_properties(simulation PROPERTIES WIN32_EXECUTABLE TRUE)
	endif()
endif()