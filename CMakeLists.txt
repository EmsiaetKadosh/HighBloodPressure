cmake_minimum_required(VERSION 3.5.0)
project(HighBloodPressure VERSION 0.1.0 LANGUAGES C CXX)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-D__CARLBEKS_CMAKE_VSCODE__)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_WIN32_EXECUTABLE true)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(${PROJECT_NAME} -Wno-microsoft-string-literal-from-predefined)
endif ()
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/source-charset:utf-8)
    add_compile_options(/execution-charset:utf-8)
endif ()
add_compile_options(/Zc:preprocessor)

include(CTest)
enable_testing()

add_executable(${PROJECT_NAME}
        src/main.cpp

        src/def.cpp
        src/utils/File.cpp
        src/utils/exception.cpp

        src/utils/Task.cpp

        src/interact/InteractManager.cpp
        src/utils/IText.cpp
        src/render/TextureManager.cpp
        src/render/dx/direct.cpp
        src/game/world/Location.cpp
        src/render/Renderer.cpp
        src/ui/Hud.cpp
        src/ui/Window.cpp

        src/game/Game.cpp

        src/game/entity/Entity.cpp
        src/game/world/World.cpp
        src/game/entity/Player.cpp

        src/render/dx/direct.cpp
)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../exenv/")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../exenv/")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../exenv/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../exenv/")

include(CPack)

