project(LCore)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)

option(LCORE_ENABLE_RECORDSTACK "Enable record stack" ON)
option(LCORE_ENABLE_ASSERT "Enable assert" ON)

set(LCORE_DEBUG OFF)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LCORE_DEBUG ON)
endif()

configure_file(
    ${CMAKE_SOURCE_DIR}/config.h.in
    ${CMAKE_SOURCE_DIR}/include/config.h
)

FILE(GLOB_RECURSE SRC_FILES src/*.c*)
FILE(GLOB_RECURSE INC_FILES include/*.h)

include_directories(include)
add_library(lcore ${SRC_FILES} ${INC_FILES})
