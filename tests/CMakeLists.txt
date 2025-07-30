
# Find and compile all the source files in the current directory

FILE(GLOB_RECURSE SRC_FILES *.cc *.cpp *.c)
foreach(SRC_FILE ${SRC_FILES})
    # convert absolute path to relative path
    file(RELATIVE_PATH SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${SRC_FILE})
    message(STATUS "Found source file: ${SRC_FILE}")
    set(TARGET_NAME ${SRC_FILE})
    string(REPLACE ".cc" "" TARGET_NAME ${TARGET_NAME})
    string(REPLACE ".c" "" TARGET_NAME ${TARGET_NAME})
    # Target name should not contain '/', replace it with '_'
    string(REPLACE "/" "_" TARGET_NAME ${TARGET_NAME})
    add_executable(${TARGET_NAME} ${SRC_FILE})
    target_link_libraries(${TARGET_NAME} lcore GTest::GTest GTest::Main)
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endforeach()



