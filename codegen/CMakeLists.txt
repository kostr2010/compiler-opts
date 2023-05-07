add_library(codegen SHARED
    codegen.cpp
)
target_link_libraries(codegen ir passes range)
