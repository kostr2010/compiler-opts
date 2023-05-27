cmake_minimum_required(VERSION 3.16)

include(cmake/cpp-paranoic-debug-flags.cmake)

project(compiler_course CXX)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(ir)
add_subdirectory(pass)
add_subdirectory(utils)
add_subdirectory(codegen)
add_subdirectory(tests)

add_executable(test test.cpp)
target_include_directories(test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(test ir marker range passes codegen)
