file(GLOB sources "*.cpp" "*.c" "../include/*.h")
set(lib_sources ${sources})

list(REMOVE_ITEM lib_sources ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_library(4xlib ${lib_sources})
add_executable(4xsim ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(4xsim 4xlib)
