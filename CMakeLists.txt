cmake_minimum_required(VERSION 3.2)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_STANDARD 99)

project(6502_emulator)

set(SRC_DIR src)
set(LIB_DIR ${SRC_DIR}/lib)
set(TEST_DIR ${SRC_DIR}/test)
set(DEPS_DIR deps)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(6502 ${TEST_DIR}/main.c ${TEST_DIR}/emulator.c ${LIB_DIR}/cpu.c ${DEPS_DIR}/cjson/cJSON.c)
target_link_libraries(6502 ${CURSES_LIBRARIES})
