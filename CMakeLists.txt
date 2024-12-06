project(LCore)
cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)

# If MSVC, enable /Zc:__cplusplus
if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

option(LCORE_ENABLE_RECORDSTACK "Enable record stack" ON)
option(LCORE_ENABLE_ASSERT "Enable assert" ON)

if (NOT DEFINED LCORE_NAMESPACE_NAME)
    set(LCORE_NAMESPACE_NAME "lcore" CACHE STRING "Namespace name")
endif()

if (LCORE_NAMESPACE_NAME STREQUAL "")
    set(LCORE_NAMESPACE_BEGIN "")
    set(LCORE_NAMESPACE_END "")
else()
    set(LCORE_NAMESPACE_BEGIN "namespace ${LCORE_NAMESPACE_NAME} {")
    set(LCORE_NAMESPACE_END "}")
endif()

set(LCORE_DEBUG OFF)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LCORE_DEBUG ON)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
)

FILE(GLOB_RECURSE SRC_FILES src/*.c*)
FILE(GLOB_RECURSE INC_FILES include/*.h)

include_directories(include)
add_library(lcore ${SRC_FILES} ${INC_FILES})
set_target_properties(lcore PROPERTIES 
    POSITION_INDEPENDENT_CODE ON
)

# add postfix if debug
set_target_properties(lcore PROPERTIES DEBUG_POSTFIX "d")

option(LCORE_ENABLE_TEST "Enable test for lcore" OFF)

if (LCORE_ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()
