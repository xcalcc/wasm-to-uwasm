cmake_minimum_required(VERSION 3.0)
project(wasm2uwasm)

set(CMAKE_CXX_STANDARD 11)

add_library(wasm_common basics.cxx winst.cpp uwinst.cpp
                        reader.cpp convertor.cpp ubh.cpp
                        executor.cxx)

target_include_directories(wasm_common PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(WABT_BASE_DIR src)

add_library(binary_reader
 ${WABT_BASE_DIR}/string-view.cc
 ${WABT_BASE_DIR}/binary-reader-logging.cc
 ${WABT_BASE_DIR}/expr-visitor.cc
 ${WABT_BASE_DIR}/hash-util.cc
 ${WABT_BASE_DIR}/option-parser.cc
 ${WABT_BASE_DIR}/binary-reader-objdump.cc
 ${WABT_BASE_DIR}/ir-util.cc
 ${WABT_BASE_DIR}/leb128.cc
 ${WABT_BASE_DIR}/filenames.cc
 ${WABT_BASE_DIR}/binary-reader-opcnt.cc
 ${WABT_BASE_DIR}/utf8.cc
 ${WABT_BASE_DIR}/ir.cc
 ${WABT_BASE_DIR}/feature.cc
 ${WABT_BASE_DIR}/stream.cc
 ${WABT_BASE_DIR}/binding-hash.cc
 ${WABT_BASE_DIR}/binary-reader-ir.cc
 ${WABT_BASE_DIR}/tracing.cc
 ${WABT_BASE_DIR}/binary.cc
 ${WABT_BASE_DIR}/binary-reader.cc
 ${WABT_BASE_DIR}/literal.cc
 ${WABT_BASE_DIR}/opcode.cc
 ${WABT_BASE_DIR}/opcode-code-table.c
 ${WABT_BASE_DIR}/common.cc
 ${WABT_BASE_DIR}/binary-reader-objdump.cc)

target_include_directories(binary_reader PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(wasm_cvt
  ${WABT_BASE_DIR}/wasm-cvt.cc
  test.cpp)

target_link_libraries(wasm_cvt PRIVATE binary_reader wasm_common)

target_include_directories(wasm_cvt PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(wasm2uwasm test.cpp)
target_compile_definitions(wasm2uwasm PRIVATE -DDEBUG)
target_link_libraries(wasm2uwasm PRIVATE wasm_common)
