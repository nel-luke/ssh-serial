# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CgvFindVersion)
cgv_find_version(Project)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(SSH-Serial VERSION ${Project_VERSION} LANGUAGES C)

configure_file("cmake/config.h.in" "${CMAKE_BINARY_DIR}/config.h" @ONLY)