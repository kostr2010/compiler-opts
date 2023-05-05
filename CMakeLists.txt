cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
                    -pedantic \
                    -pedantic-errors \
                    -Wall \
                    -Wfloat-equal \
                    -Wextra \
                    -Winline \
                    -Winvalid-pch \
                    -Wcast-align \
                    -Wcast-qual \
                    -Wdisabled-optimization \
                    -Wformat=2 \
                    -Winit-self \
                    -Wlogical-op \
                    -Wmissing-declarations \
                    -Wold-style-cast \
                    -Woverloaded-virtual \
                    -Wredundant-decls \
                    -Wshadow \
                    -Wsign-promo \
                    -Wstrict-null-sentinel \
                    -Wstrict-overflow=5 \
                    -Walloc-zero \
                    -Wimplicit-fallthrough \
                    -Wunused \
                    -Wuninitialized \
                    -Wno-invalid-memory-model \
                    -Wmaybe-uninitialized \
                    -Wswitch \
                    -Werror"
    CACHE STRING "Default CXX options" FORCE)
set(CMAKE_CXX_STANDARD 20)

project(compiler_course CXX)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(ir)
add_subdirectory(pass)
add_subdirectory(utils)
add_subdirectory(codegen)
add_subdirectory(tests)
add_subdirectory(third-party)

add_executable(test test.cpp)
target_include_directories(test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(test ir)
