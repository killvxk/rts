cmake_minimum_required (VERSION 3.8)
set(CMAKE_VERBOSE_MAKEFILE 1)
file(GLOB_RECURSE RTS_SOURCES "*.c*")

add_executable (rts ${RTS_SOURCES})
