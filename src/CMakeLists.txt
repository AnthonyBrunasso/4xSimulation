project(4xSimulation)                                                                                         
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")                                                          
include_directories("../include")      
include_directories("../flatbuffers/include")

file(GLOB sources "*.cpp" "*.c" "../include/*.h")
set(lib_sources ${sources})
list(REMOVE_ITEM lib_sources ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
list(REMOVE_ITEM lib_sources ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

# Request compilation of .fbs files
if(NOT FLATBUFFERS_FLATC_EXECUTABLE)                                                                          
    set(FLATBUFFERS_FLATC_EXECUTABLE $<TARGET_FILE:flatc>)
  MESSAGE( "Set flatbuffers executable." )                                                                    
endif()                                                                                                       
compile_flatbuffers_schema_to_cpp(generated/step.fbs)  
# Add the output path to the include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated/)

add_library(4xlib ${lib_sources} ${CMAKE_CURRENT_BINARY_DIR}/generated/step_generated.h )
add_executable(4xsim ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
add_executable(4xtest ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
target_link_libraries(4xtest flatbuffers)

target_link_libraries(4xsim 4xlib)
target_link_libraries(4xsim flatbuffers)
