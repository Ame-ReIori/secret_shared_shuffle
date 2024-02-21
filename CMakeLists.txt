cmake_minimum_required(VERSION 3.16)

if (DEFINED DEBUG)
  add_definitions(-DDEBUG)
endif()

project(ss_shuffle)

add_subdirectory(src)