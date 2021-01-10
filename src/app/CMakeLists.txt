add_executable(matching_engine
    main.cpp)

target_link_libraries(matching_engine
    PRIVATE
    project_options
    project_warnings
    libmatching_engine)
