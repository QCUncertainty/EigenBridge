cmake_minimum_required(VERSION 3.15)

project(
    eigenbridge
    VERSION "${SKBUILD_PROJECT_VERSION}"
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
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "cxx/src/${PROJECT_NAME}/*.cpp")
file(GLOB_RECURSE PYTHON_FILES CONFIGURE_DEPENDS "python/*")

# Build the library
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(BUILD_SHARED_LIBS ON)
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

# Add the Python module
pybind11_add_module(${PROJECT_NAME}_core
    ${SOURCE_FILES}
    ${PYTHON_FILES}
)
target_link_libraries(${PROJECT_NAME}_core PUBLIC LAPACK::LAPACK lapacke)
target_link_directories(${PROJECT_NAME}_core PUBLIC ${LAPACKE_LIBRARY_DIRS})
target_include_directories(${PROJECT_NAME}_core PRIVATE ${LAPACKE_INCLUDE_DIRS})

# Install the Python module
install(TARGETS ${PROJECT_NAME}_core
    COMPONENT python
    LIBRARY DESTINATION ${PROJECT_NAME}
)
