add_library(catch_main STATIC test_main.cpp)
target_link_libraries(catch_main PRIVATE project_options)

add_executable(test_matching_engine
    test_matching_engine.cpp)
target_link_libraries(test_matching_engine
    PRIVATE
    libmatching_engine
    project_warnings
    project_options
    catch_main)

add_test(NAME tests
         COMMAND test_matching_engine)
