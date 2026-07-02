cmake_minimum_required(VERSION 3.15)

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

# Files
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/${PROJECT_NAME}/*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "include/${PROJECT_NAME}/*.hpp")
file(GLOB_RECURSE PYTHON_FILES CONFIGURE_DEPENDS "python/*.cpp" "python/*.hpp")

# Build the library
if(${SKBUILD})
    # Python
    message(STATUS "Building Python bindings for ${PROJECT_NAME}...")
    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
    set(BUILD_SHARED_LIBS ON)
    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 CONFIG REQUIRED)

    # Add the Python module
    pybind11_add_module(${PROJECT_NAME}
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${PYTHON_FILES}
    )
    target_link_libraries(${PROJECT_NAME} PUBLIC LAPACK::LAPACK lapacke)

    # Install the Python module
    install(TARGETS ${PROJECT_NAME}
        COMPONENT python
        LIBRARY DESTINATION .
    )
else()
    # C++
    message(STATUS "Building CPP Library ${PROJECT_NAME}...")

    # Create the library target
    add_library(${PROJECT_NAME} ${SOURCE_FILES})
    target_link_libraries(${PROJECT_NAME} LAPACK::LAPACK lapacke)
    target_include_directories(
        ${PROJECT_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    # Install the library
    include(cmake/install_target.cmake)
    install_library(
        ${PROJECT_NAME}
        "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}"
    )

    # Build the unit tests
    if(${BUILD_TESTING})
        include(CTest)

        set(test_target_name "unit_test_${PROJECT_NAME}")
        set(test_dir "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_tests")
        file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS ${test_dir}/*.cpp)

        add_executable(${test_target_name} ${TEST_FILES})
        include(cmake/FetchCatch2.cmake)
        target_link_libraries(
            ${test_target_name} PRIVATE Catch2::Catch2WithMain ${PROJECT_NAME}
        )

        add_test(
            NAME ${test_target_name}
            COMMAND ${test_target_name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()
endif()
