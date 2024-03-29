find_package(GTest REQUIRED)

set(TESTS

    # passes
    rpo_test.cpp
    dom_tree_test.cpp
    loop_analysis_test.cpp
    basic_test.cpp

    dce_test.cpp
    peepholes_test.cpp
    inlining_test.cpp
    check_elimination_test.cpp
    linear_order_test.cpp
    liveness_analysis_test.cpp
    regalloc_test.cpp

    # utils
    range_test.cpp
    type_sequence_test.cpp
    type_helpers_test.cpp
)

add_executable(gtests ${TESTS})
target_link_libraries(gtests ir GTest::gtest GTest::gtest_main pthread)
target_include_directories(gtests PRIVATE ${PROJECT_SOURCE_DIR}/ir ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
