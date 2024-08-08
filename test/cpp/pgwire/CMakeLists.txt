add_executable(pgwire-test
    main.cpp
    utils.cpp
)
target_link_libraries(pgwire-test PRIVATE catch2 pgwire)
