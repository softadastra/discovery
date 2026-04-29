cmake_minimum_required(VERSION 3.20)

set(SOFTADASTRA_DISCOVERY_EXAMPLES
    discovery_minimal
    discovery_announcer
    discovery_listener
    discovery_roundtrip_demo
    discovery_codec
    discovery_registry
    discovery_service
    discovery_engine_setup
)

foreach(example_name IN LISTS SOFTADASTRA_DISCOVERY_EXAMPLES)
    set(example_source "${CMAKE_CURRENT_SOURCE_DIR}/${example_name}.cpp")

    if(EXISTS "${example_source}")
        add_executable(${example_name} "${example_source}")

        target_compile_features(${example_name} PRIVATE cxx_std_20)

        target_link_libraries(${example_name}
            PRIVATE
                softadastra_discovery
        )

        target_include_directories(${example_name}
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/../include
        )

        if(MSVC)
            target_compile_options(${example_name} PRIVATE /W4 /permissive-)
        else()
            target_compile_options(${example_name} PRIVATE -Wall -Wextra -Wpedantic)
        endif()

        set_target_properties(${example_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
        )

        message(STATUS "[discovery/examples] enabled: ${example_name}")
    else()
        message(WARNING "[discovery/examples] missing source: ${example_source}")
    endif()
endforeach()
