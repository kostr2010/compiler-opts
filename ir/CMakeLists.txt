set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -Wextra -g")

add_library(ir SHARED
    bb.cpp
    graph_builder.cpp
    graph.cpp
    inst.cpp
    metadata.cpp
)
