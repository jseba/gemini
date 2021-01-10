cmake_minimum_required(VERSION 3.10)

project(
    gemini
    VERSION 1.0
    LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT_CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as non was specified")
    set(CMAKE_BUILD_TYPE
        RelWithDebInfo
        CACHE STRING "Choost the type of build" FORCE)
    set_property(
        CACHE CMAKE_BUILD_TYPE
        PROPERTY STRINGS
        "Debug"
        "Release"
        "RelWithDebInfo"
        "MinSizeRel")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

add_subdirectory(lib)
add_subdirectory(app)

option(ENABLE_TESTS "Enable tests" ON)
if(ENABLE_TESTS)
    enable_testing()
    message("Building tests")
    add_subdirectory(test)
endif()
