cmake_minimum_required(VERSION 3.14)

# Set Project and Version
project(
    eigensolverapi
    VERSION "0.0.0"
    LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 17)

# Options
option(BUILD_TESTING "Whether or not to build the unit tests" OFF)

# Dependencies
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(LAPACK REQUIRED)
find_package(LAPACKE REQUIRED)

# Build the library
file(GLOB_RECURSE PROJECT_HEADER_FILES CONFIGURE_DEPENDS
     include/${PROJECT_NAME}/*.hpp
)
file(GLOB_RECURSE PROJECT_SOURCE_FILES CONFIGURE_DEPENDS
     src/${PROJECT_NAME}/*.cpp
)
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} LAPACK::LAPACK lapacke)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>
)

if(${BUILD_TESTING})
    include(CTest)
    include(FetchContent)

    FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2
        GIT_TAG "v3.11.0"
    )
    FetchContent_MakeAvailable(catch2)

    set(test_target_name "unit_test_${PROJECT_NAME}")
    set(test_dir "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_tests")

    file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS ${test_dir}/*.cpp)

    add_executable(${test_target_name} ${TEST_FILES})

    target_link_libraries(
        ${test_target_name} PRIVATE Catch2::Catch2WithMain ${PROJECT_NAME}
    )

    add_test(
        NAME ${test_target_name}
        COMMAND ${test_target_name}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

endif()
