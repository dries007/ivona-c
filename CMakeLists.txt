cmake_minimum_required(VERSION 3.3)
project(ivona)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")

set(SOURCE_FILES_C main.c ivona.c ivona.h)
add_executable(ivona ${SOURCE_FILES_C})
target_link_libraries(ivona -lcrypto -lcurl)
