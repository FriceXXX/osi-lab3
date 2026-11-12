cmake_minimum_required(VERSION 3.10)
project(Lab3_Determinant VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
else()
    add_compile_options(-Wall -Wextra -O2)
    link_libraries(pthread)
endif()

add_executable(determinant main.cpp matrix.cpp)
target_include_directories(determinant PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})