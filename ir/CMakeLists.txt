set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -Wextra -g")

set(PASSES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/passes/)

add_library(passes SHARED
    ${PASSES_DIR}/rpo.cpp
    ${PASSES_DIR}/po.cpp
    ${PASSES_DIR}/dfs.cpp
    ${PASSES_DIR}/bfs.cpp
    ${PASSES_DIR}/dom_tree.cpp
    ${PASSES_DIR}/inlining.cpp
    ${PASSES_DIR}/loop_analysis.cpp
    ${PASSES_DIR}/check_elimination.cpp
    ${PASSES_DIR}/linear_order.cpp
    ${PASSES_DIR}/liveness_analysis.cpp
    ${PASSES_DIR}/peepholes.cpp
    ${PASSES_DIR}/dce.cpp
    ${PASSES_DIR}/dbe.cpp
)

target_include_directories(passes PUBLIC ${CMAKE_SOURCE_DIR}/utils ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(passes marker)

add_library(ir SHARED
    bb.cpp
    graph_builder.cpp
    graph_visitor.cpp
    graph.cpp
    inst_.cpp
    loop.cpp
    analyser.cpp
)

target_link_libraries(ir passes marker)
target_include_directories(ir PUBLIC ${CMAKE_SOURCE_DIR}/utils ${CMAKE_CURRENT_SOURCE_DIR})
