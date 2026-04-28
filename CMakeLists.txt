cmake_minimum_required(VERSION 3.20)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(softadastra_discovery
        VERSION 0.1.0
        DESCRIPTION "Softadastra Discovery - Peer discovery layer"
        LANGUAGES CXX
    )
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ========================
# Standalone fallback options
# ========================
if(NOT DEFINED SOFTADASTRA_WARNINGS_AS_ERRORS)
    option(SOFTADASTRA_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
endif()

if(SOFTADASTRA_UMBRELLA_BUILD)
    set(SOFTADASTRA_DISCOVERY_BUILD_TESTS ${SOFTADASTRA_BUILD_TESTS})
else()
    option(SOFTADASTRA_DISCOVERY_BUILD_TESTS "Build discovery tests" OFF)
endif()

option(SOFTADASTRA_DISCOVERY_BUILD_EXAMPLES "Build discovery examples" OFF)

# ========================
# Dependencies
# ========================
if(NOT SOFTADASTRA_UMBRELLA_BUILD)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.vix/vix_deps.cmake")
        include("${CMAKE_CURRENT_SOURCE_DIR}/.vix/vix_deps.cmake")
    endif()
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

set(SOFTADASTRA_DISCOVERY_PLATFORM_SOURCES)

if(UNIX)
    file(GLOB_RECURSE SOFTADASTRA_DISCOVERY_POSIX_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/linux/*.cpp"
    )
    list(APPEND SOFTADASTRA_DISCOVERY_PLATFORM_SOURCES
        ${SOFTADASTRA_DISCOVERY_POSIX_SOURCES}
    )
endif()

if(WIN32)
    file(GLOB_RECURSE SOFTADASTRA_DISCOVERY_WINDOWS_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/windows/*.cpp"
    )
    list(APPEND SOFTADASTRA_DISCOVERY_PLATFORM_SOURCES
        ${SOFTADASTRA_DISCOVERY_WINDOWS_SOURCES}
    )
endif()

# ========================
# Library
# ========================
add_library(softadastra_discovery
    ${SOFTADASTRA_DISCOVERY_SOURCES}
    ${SOFTADASTRA_DISCOVERY_PLATFORM_SOURCES}
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
    if(SOFTADASTRA_WARNINGS_AS_ERRORS)
        target_compile_options(softadastra_discovery PRIVATE /WX)
    endif()
else()
    target_compile_options(softadastra_discovery PRIVATE -Wall -Wextra -Wpedantic)
    if(SOFTADASTRA_WARNINGS_AS_ERRORS)
        target_compile_options(softadastra_discovery PRIVATE -Werror)
    endif()
endif()

# ========================
# Platform definitions
# ========================
if(UNIX)
    target_compile_definitions(softadastra_discovery PRIVATE SOFTADASTRA_DISCOVERY_POSIX)
endif()

if(UNIX AND NOT APPLE)
    target_compile_definitions(softadastra_discovery PRIVATE SOFTADASTRA_DISCOVERY_LINUX)
elseif(APPLE)
    target_compile_definitions(softadastra_discovery PRIVATE SOFTADASTRA_DISCOVERY_MAC)
elseif(WIN32)
    target_compile_definitions(softadastra_discovery PRIVATE SOFTADASTRA_DISCOVERY_WINDOWS)
endif()

# ========================
# Target properties
# ========================
set_target_properties(softadastra_discovery PROPERTIES
    OUTPUT_NAME softadastra_discovery
    VERSION 0.1.0
    SOVERSION 0
    EXPORT_NAME discovery
)

# ========================
# Install
# ========================
install(
    TARGETS softadastra_discovery
    EXPORT softadastraDiscoveryTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT softadastraDiscoveryTargets
    FILE softadastraDiscoveryTargets.cmake
    NAMESPACE softadastra::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/softadastra_discovery"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/softadastra_discoveryConfigVersion.cmake"
    VERSION 0.1.0
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/softadastra_discoveryConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/softadastra_discoveryConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/softadastra_discovery"
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/softadastra_discoveryConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/softadastra_discoveryConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/softadastra_discovery"
)

# ========================
# Examples
# ========================
if(SOFTADASTRA_DISCOVERY_BUILD_EXAMPLES AND NOT SOFTADASTRA_UMBRELLA_BUILD)
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
message(STATUS "softadastra::discovery configured (0.1.0)")
message(STATUS "Transport target: ${SOFTADASTRA_DISCOVERY_TRANSPORT_TARGET}")
message(STATUS "Include dir:      ${CMAKE_CURRENT_SOURCE_DIR}/include")
message(STATUS "Build tests:      ${SOFTADASTRA_DISCOVERY_BUILD_TESTS}")
message(STATUS "Build examples:   ${SOFTADASTRA_DISCOVERY_BUILD_EXAMPLES}")
message(STATUS "Warnings=error:   ${SOFTADASTRA_WARNINGS_AS_ERRORS}")
message(STATUS "Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "------------------------------------------------------")
