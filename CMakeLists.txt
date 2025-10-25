cmake_minimum_required(VERSION 3.30)
project(HighBloodPressure_DX) ## 此处是项目名

message("[ek] PROJECT_NAME is ${PROJECT_NAME}")
message("[ek] Project ${PROJECT_NAME} CMake building")
set(HBP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message("[ek] Root directory of HBP: ${HBP_SRC_DIR}")

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_WIN32_EXECUTABLE true)
set(CMAKE_BUILD_TYPE Debug)

include(CTest)
enable_testing()

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${HBP_SRC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE "${HBP_SRC_DIR}/../DirectX-lib/include")
add_subdirectory(src)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../exenv/")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../exenv/")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../exenv/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../exenv/")

include(CPack)
