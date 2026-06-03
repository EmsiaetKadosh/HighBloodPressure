cmake_minimum_required(VERSION 3.30)
project(HighBloodPressure_DX) ## 此处是项目名

message("[ek] PROJECT_NAME is ${PROJECT_NAME}")
message("[ek] Project ${PROJECT_NAME} CMake building")
set(HBP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message("[ek] Root directory of HBP: ${HBP_SRC_DIR}")
execute_process(COMMAND chcp 65001)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_VS_INCLUDE_INTERNAL_TOOL OFF) # 包含列表全遍历控制
set(CMAKE_WIN32_EXECUTABLE true)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_DEFAULT_ENCODING_UTF8 ON)  # CMake 3.20+
add_definitions(-DCMAKE_DEFAULT_ENCODING_UTF8=ON)
# 告诉 CMake 使用 UTF-8 输出
if(POLICY CMP0135)
	cmake_policy(SET CMP0135 NEW)  # CMake 3.20+ 支持更好的UTF-8处理
	message(SET CMP0135 NEW)
endif()

include(CTest)
enable_testing()

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${HBP_SRC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE "${HBP_SRC_DIR}/includes")
add_subdirectory(src)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../exenv/")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../exenv/")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../exenv/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../exenv/")

include(CPack)
