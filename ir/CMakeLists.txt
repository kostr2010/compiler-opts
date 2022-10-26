set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -Wextra -g")

set(PASSES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/passes/)

add_library(ir SHARED
    bb.cpp
    graph_builder.cpp
    graph.cpp
    inst.cpp
    mark.cpp
    analyser.cpp
    ${PASSES_DIR}/rpo.cpp
    ${PASSES_DIR}/dfs.cpp
    ${PASSES_DIR}/bfs.cpp
    ${PASSES_DIR}/dom_tree_slow.cpp
)

target_include_directories(ir PUBLIC ${CMAKE_SOURCE_DIR}/utils ${CMAKE_CURRENT_SOURCE_DIR})

# add_subdirectory(passes)