set(TESTS 
    rpo_test.cpp
    dom_tree_test.cpp
    loop_analysis_test.cpp
    basic_test.cpp
    main.cpp
    dce_test.cpp
    peepholes_test.cpp
    )

add_executable(gtests ${TESTS})
target_link_libraries(gtests ir gtest gtest_main pthread)
target_include_directories(gtests PRIVATE ${PROJECT_SOURCE_DIR}/ir ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
