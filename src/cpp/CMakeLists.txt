# FILE(GLOB SRC *.cpp)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIX -D_LINUX=1 -DCMDLINE=1")

get_filename_component(project_BIN_DIR bin ABSOLUTE)
get_filename_component(project_LIB_DIR lib ABSOLUTE)

include_directories(.)

add_subdirectory(minnow_comms)
add_subdirectory(minnow_drivers)
add_subdirectory(minnow_apps)
