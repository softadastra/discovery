add_executable(softadastra_discovery_test_basic
    discovery_test_basic.cpp
)

target_link_libraries(softadastra_discovery_test_basic
    PRIVATE
        softadastra::discovery
)

add_test(
    NAME softadastra_discovery_test_basic
    COMMAND softadastra_discovery_test_basic
)
