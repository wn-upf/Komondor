cmake_minimum_required(VERSION 3.10)
project(Komondor)

option(INSTALL_COMPCPP "Install cxx binary" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
add_subdirectory(CompCpp)
add_subdirectory(src)
