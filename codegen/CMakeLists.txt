add_library(codegen SHARED
    linear_scan.cpp
)
target_link_libraries(codegen ir passes range)
