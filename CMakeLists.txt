cmake_minimum_required(VERSION 3.16)

# -flto + -Wlto-type-mismatch
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
                    -pedantic \
                    -pedantic-errors \
                    -Wall \
                    -Wextra \
                    -Winline \
                    -Winvalid-pch \
                    -Wcast-align \
                    -Wcast-qual \
                    -Wdisabled-optimization \
                    -Wlogical-op \
                    -Wold-style-cast \
                    -Woverloaded-virtual \
                    -Wredundant-decls \
                    -Wsign-promo \
                    -Wstrict-null-sentinel \
                    -Wstrict-overflow=5 \
                    -Walloc-zero \
                    -Wimplicit-fallthrough \
                    -Wunused \
                    -Winvalid-memory-model \
                    -Wmaybe-uninitialized \
                    -Warray-bounds=2 \
                    -Warray-parameter=2 \
                    -Wswitch \
                    -Werror \
                    -Winvalid-constexpr \
                    -Winvalid-imported-macros \
                    -Wnoexcept \
                    -Wplacement-new=2 \
                    -Waligned-new \
                    -Wuse-after-free=3 \
                    -Wdouble-promotion \
                    -Wformat=2 \
                    -Wuninitialized \
                    -Winit-self \
                    -Wnull-dereference \
                    -Winfinite-recursion \
                    -Wimplicit-fallthrough=5 \
                    -Wsuggest-attribute=const \
                    -Wduplicated-branches \
                    -Wduplicated-cond \
                    -Wfloat-equal \
                    -Wshadow \
                    -Wunsafe-loop-optimizations \
                    -Wcast-align=strict \
                    -Wcast-function-type \
                    -Wwrite-strings \
                    -Wconversion \
                    -Wparentheses \
                    -Wsign-compare \
                    -Wmissing-declarations\
                    -Wmissing-field-initializers \
                    -Wmissing-requires \
                    -Wmissing-template-keyword \
                    -Wredundant-decls \
                    -Woverlength-strings \
                    -fstack-protector \
                    -fstack-protector-all \
                    -fstack-protector-strong \
                    -fstack-clash-protection \
                    -Wstack-protector \
                    -faligned-new \
                    -fstrict-enums \
                    -fdiagnostics-color=always \
                    -fdiagnostics-show-template-tree \
                    -fsanitize=undefined \
                    -fsanitize=leak \
                    -fsanitize-address-use-after-scope \
                    -Weffc++ \
                    -Wnon-virtual-dtor \
                    -ggdb3 \
                    -fsanitize=address"
    CACHE STRING "Default CXX options" FORCE)

set(CMAKE_CXX_COMPILER "g++-13")
set(CMAKE_CXX_STANDARD 20)

project(compiler_course CXX)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(ir)
add_subdirectory(pass)
add_subdirectory(utils)
add_subdirectory(codegen)
add_subdirectory(tests)

# add_subdirectory(third-party)
add_executable(test test.cpp)
target_include_directories(test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(test ir marker range passes codegen)
