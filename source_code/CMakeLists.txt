cmake_minimum_required(VERSION 3.31)
project(Orgulous)

add_executable(orgulous main.cpp)
target_compile_options(orgulous PUBLIC -std=c++1y -Wall -Wfloat-conversion)
target_include_directories(orgulous PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_link_libraries(orgulous ${CMAKE_SOURCE_DIR}/librmlui.a)
