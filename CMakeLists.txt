cmake_minimum_required(VERSION 3.8)

project(vkst LANGUAGES C CXX)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(cmake/CompilerConfig.cmake)

add_subdirectory(ext)
add_subdirectory(src)
