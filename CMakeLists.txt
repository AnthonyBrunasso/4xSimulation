cmake_minimum_required (VERSION 2.6)

option(4X_CHECK_INCLUDES "Enable iwyu outputs." OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
SET(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "disable flatbuffer tests")
SET(FLATBUFFERS_BUILD_FLATHASH OFF CACHE BOOL "disable flathash")
add_subdirectory("flatbuffers")
add_subdirectory("src")
