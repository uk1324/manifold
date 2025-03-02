add_executable(game2 "main.cpp" "MainLoop.cpp" "Game.cpp" "GameRenderer.cpp")

target_link_libraries(game2 PUBLIC engine)

target_compile_features(game2 PUBLIC cxx_std_23)
set_target_properties(game2 PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(game2 PUBLIC "../" "../engine/dependencies/")

include("../engine/codeGenTool/targetAddGenerated.cmake")

target_link_libraries(game2 PUBLIC gfx2d)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/../engine/dependencies/freetype.dll" 
	"${CMAKE_CURRENT_SOURCE_DIR}/../freetype.dll" 
	COPYONLY)

if (WIN32)
	target_link_libraries(game2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../engine/dependencies/freetype.lib")
else()
	target_link_libraries(game2 PUBLIC freetypeStaticRelease)
endif()


# enumerator 'identifier' in switch of enum 'enumeration' is not handled
target_compile_options(game2 PRIVATE /we4062)

targetAddGenerated(game2 ${CMAKE_CURRENT_SOURCE_DIR})

# If this is on, then the console logs won't show even when launched from console.
if (FINAL_RELEASE)
	if (WIN32)
		set_target_properties(game2 PROPERTIES WIN32_EXECUTABLE TRUE)
	endif()
endif()