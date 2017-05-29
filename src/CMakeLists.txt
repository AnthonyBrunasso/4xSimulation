project(4xSimulation)                                                                                         
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")                                                          
include_directories("../include")      
include_directories("../flatbuffers/include")
include_directories("../nn_cpp/include")

file(GLOB console_sources "console/*.cpp")
file(GLOB learn_sources "learn/*.cpp")
file(GLOB lib_sources "*.cpp" "*.c" "../include/*.h")
list(REMOVE_ITEM lib_sources ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

# Request compilation of .fbs files
if(NOT FLATBUFFERS_FLATC_EXECUTABLE)                                                                          
    set(FLATBUFFERS_FLATC_EXECUTABLE $<TARGET_FILE:flatc>)
  MESSAGE( "Set flatbuffers executable." )                                                                    
endif()                                                                                                       
# Make our own builder
# unused flags, for now: --gen-object-api --gen-mutable
function(fourx_compile_flatbuffers_schema_to_cpp SRC_FBS)
  get_filename_component(SRC_FBS_DIR ${SRC_FBS} PATH)
  string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER ${SRC_FBS})
  add_custom_command(
    OUTPUT ${GEN_HEADER}
    COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -c --no-includes 
            --no-prefix --scoped-enums
            -o "${SRC_FBS_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}"
    DEPENDS flatc)
endfunction()

fourx_compile_flatbuffers_schema_to_cpp(generated/step.fbs)  
# Add the output path to the include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated/)

add_library(4xlib ${lib_sources} ${CMAKE_CURRENT_BINARY_DIR}/generated/step_generated.h )
add_executable(4xsim ${console_sources})
add_executable(4xlearn ${learn_sources})
add_executable(4xtest ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp ${CMAKE_CURRENT_BINARY_DIR}/generated/step_generated.h )

target_link_libraries(4xtest flatbuffers)
target_link_libraries(4xsim 4xlib flatbuffers)
target_link_libraries(4xlearn 4xlib flatbuffers)


if (4X_CHECK_INCLUDES)
  find_program(iwyu_path NAMES include-what-you-use iwyu)
  if(NOT iwyu_path)
    message(FATAL_ERROR "Could not find the program include-what-you-use")
  endif()
  set_property(TARGET 4xlib PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path}) 
endif()

