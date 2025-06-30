cmake_minimum_required(VERSION 3.5.0)
project(HighBloodPressure VERSION 0.1.0 LANGUAGES C CXX)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-D__CARLBEKS_CMAKE_VSCODE__)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_WIN32_EXECUTABLE true)
set(CMAKE_BUILD_TYPE Debug)

add_compile_options(/source-charset:utf-8)
add_compile_options(/execution-charset:utf-8)
add_compile_options(/Zc:preprocessor)

include(CTest)
enable_testing()

if (true)
	set(HBP_RENDER_SYSTEM gdi)
	set(HBP_RENDER_SYSTEM_FILES
		src/render/gdi/font.cpp
		src/render/gdi/texture.cpp
		src/render/gdi/renderer.cpp
	)
else ()
	set(HBP_RENDER_SYSTEM dx)
	set(HBP_RENDER_SYSTEM_FILES
		src/render/dx/font.cpp
		src/render/dx/direct.cpp
	)
	add_definitions(-D__CARLBEKS_USE_DX__)
endif ()

set(HBP_STANDARD_FILES
	src/main.cpp
	src/global.cpp

	src/def.cpp
	# math
	src/utils/File.cpp
	src/utils/exception.cpp
	# Chars

	src/utils/gc.cpp
	# utils
	src/utils/Task.cpp
	# hbp
	src/game/Animation.cpp

	src/interact/InteractManager.cpp
	src/utils/IText.cpp
	src/game/Description.cpp
	src/game/world/Location.cpp
	src/render/Pose.cpp
	src/render/TextureManager.cpp
	src/render/Renderer.cpp
	src/ui/Hud.cpp
	src/ui/TextEditor.cpp
	src/ui/Window.cpp

	src/game/Game.cpp

	# xWindows
	# gameDef
	src/game/entity/Entity.cpp
	src/game/world/Block.cpp
	src/game/world/World.cpp
	src/game/entity/Player.cpp

	# xBlocks
)

add_executable(${PROJECT_NAME}
	${HBP_STANDARD_FILES}
	${HBP_RENDER_SYSTEM_FILES}
)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../exenv/")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../exenv/")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../exenv/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../exenv/")

include(CPack)

