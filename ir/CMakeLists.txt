add_library(ir SHARED
    bb.cpp
    graph_builder.cpp
    graph_visitor.cpp
    graph.cpp
    inst.cpp
    loop.cpp
)
target_link_libraries(ir passes marker)
