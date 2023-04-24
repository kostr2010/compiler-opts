add_library(marker SHARED
    markable.cpp
    marker_factory.cpp
    marker.cpp
)

target_include_directories(marker PUBLIC ${CMAKE_SOURCE_DIR}/utils)
