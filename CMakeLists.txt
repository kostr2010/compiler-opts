cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror" CACHE STRING "Default CXX options" FORCE)
set(CMAKE_CXX_STANDARD 20)

project(compiler_course CXX)

add_subdirectory(ir)
add_subdirectory(tests)
add_subdirectory(third-party)

add_executable(test test.cpp)
target_include_directories(test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(test ir)
