cmake_minimum_required(VERSION 3.20)

project(softadastra_discovery
    VERSION 0.1.0
    DESCRIPTION "Softadastra Discovery - Peer discovery layer"
    LANGUAGES CXX
)

include(GNUInstallDirs)

# ========================
# Options
# ========================
option(SOFTADASTRA_DISCOVERY_BUILD_TESTS "Build discovery tests" ON)
option(SOFTADASTRA_DISCOVERY_BUILD_EXAMPLES "Build discovery examples" ON)

# ========================
# Global settings
# ========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ========================
# Dependencies
# ========================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.vix/vix_deps.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/.vix/vix_deps.cmake")
endif()

if(NOT TARGET softadastra::transport AND NOT TARGET softadastra_transport)
    find_package(softadastra_transport CONFIG QUIET)
    find_package(transport CONFIG QUIET)
endif()

set(SOFTADASTRA_DISCOVERY_TRANSPORT_TARGET "")
if(TARGET softadastra::transport)
    set(SOFTADASTRA_DISCOVERY_TRANSPORT_TARGET softadastra::transport)
elseif(TARGET softadastra_transport)
    set(SOFTADASTRA_DISCOVERY_TRANSPORT_TARGET softadastra_transport)
endif()

if(SOFTADASTRA_DISCOVERY_TRANSPORT_TARGET STREQUAL "")
    message(FATAL_ERROR
        "softadastra transport target not found.\n"
        "Install dependencies with Vix:\n"
        "  vix add @softadastra/transport\n"
        "  vix install"
    )
endif()

# ========================
# Sources
# ========================
file(GLOB_RECURSE SOFTADASTRA_DISCOVERY_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# ========================
# Library
# ========================
add_library(softadastra_discovery
    ${SOFTADASTRA_DISCOVERY_SOURCES}
)

add_library(softadastra::discovery ALIAS softadastra_discovery)

target_compile_features(softadastra_discovery PUBLIC cxx_std_20)

target_link_libraries(softadastra_discovery
    PUBLIC
        ${SOFTADASTRA_DISCOVERY_TRANSPORT_TARGET}
)

# ========================
# Include directories
# ========================
target_include_directories(softadastra_discovery
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ========================
# Compiler options
# ========================
if(MSVC)
    target_compile_options(softadastra_discovery PRIVATE /W4 /permissive-)
else()
    target_compile_options(softadastra_discovery PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ========================
# Target properties
# ========================
set_target_properties(softadastra_discovery PROPERTIES
    OUTPUT_NAME softadastra_discovery
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    EXPORT_NAME discovery
)

# ========================
# Examples
# ========================
if(SOFTADASTRA_DISCOVERY_BUILD_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
        add_subdirectory(examples)
    else()
        message(STATUS "[discovery] examples/ missing CMakeLists.txt -> skipping")
    endif()
endif()

# ========================
# Tests
# ========================
if(SOFTADASTRA_DISCOVERY_BUILD_TESTS)
    include(CTest)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    else()
        message(STATUS "[discovery] tests/ missing CMakeLists.txt -> skipping")
    endif()
endif()

# ========================
# Summary
# ========================
message(STATUS "------------------------------------------------------")
message(STATUS "softadastra::discovery configured (${PROJECT_VERSION})")
message(STATUS "Transport target: ${SOFTADASTRA_DISCOVERY_TRANSPORT_TARGET}")
message(STATUS "Include dir:      ${CMAKE_CURRENT_SOURCE_DIR}/include")
message(STATUS "Build tests:      ${SOFTADASTRA_DISCOVERY_BUILD_TESTS}")
message(STATUS "Build examples:   ${SOFTADASTRA_DISCOVERY_BUILD_EXAMPLES}")
message(STATUS "Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "------------------------------------------------------")
