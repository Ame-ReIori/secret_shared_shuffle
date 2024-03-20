cmake_minimum_required(VERSION 3.16)

project(ss_shuffle)

string(ASCII 27 Esc)
set(E "${Esc}[m")
set(R "${Esc}[31m")
set(G "${Esc}[32m")

include(cmake/sssPreamble.cmake)
include(cmake/buildOptions.cmake)
include(cmake/findDependancies.cmake)

add_subdirectory(src)