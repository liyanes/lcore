project(LCore)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)

# Enable C++ 20 modules
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")

option(LCORE_ENABLE_RECORDSTACK "Enable record stack" ON)
option(LCORE_ENABLE_ASSERT "Enable assert" ON)

if (NOT DEFINED LCORE_NAMESAPCE_NAME)
    set(LCORE_NAMESAPCE_NAME "lcore" CACHE STRING "Namespace name")
endif()

if (LCORE_NAMESAPCE_NAME STREQUAL "")
    set(LCORE_NAMESAPCE_BEGIN "")
    set(LCORE_NAMESAPCE_END "")
else()
    set(LCORE_NAMESAPCE_BEGIN "namespace ${LCORE_NAMESAPCE_NAME} {")
    set(LCORE_NAMESAPCE_END "}")
endif()

set(LCORE_DEBUG OFF)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LCORE_DEBUG ON)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h
)

FILE(GLOB_RECURSE SRC_FILES src/*.c*)
FILE(GLOB_RECURSE INC_FILES include/*.h)

include_directories(include)
add_library(lcore ${SRC_FILES} ${INC_FILES})

option(LCORE_ENABLE_TEST "Enable test for lcore" ON)

if (LCORE_ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()
