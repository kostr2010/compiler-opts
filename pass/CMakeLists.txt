add_library(passes SHARED
    rpo.cpp
    po.cpp
    dfs.cpp
    bfs.cpp
    dom_tree.cpp
    inlining.cpp
    loop_analysis.cpp
    check_elimination.cpp
    linear_order.cpp
    linear_scan.cpp
    liveness_analysis.cpp
    peepholes.cpp
    dce.cpp
    dbe.cpp
    pass_manager.cpp
)
target_link_libraries(passes marker range)
