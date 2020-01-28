# FILE(GLOB SRC *.cpp)

cmake_minimum_required(VERSION 2.6)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIX -D_LINUX=1 -DCMDLINE=1")

get_filename_component(project_BIN_DIR bin ABSOLUTE)
get_filename_component(project_LIB_DIR lib ABSOLUTE)

add_subdirectory(minnow_comms)
add_subdirectory(minnow_drivers)

### metà App Depth
SET(SRC
  app_depth.cpp
)
ADD_EXECUTABLE(app_depth ${SRC})
TARGET_LINK_LIBRARIES(app_depth -lminnow_app -lyaml-cpp -lzmq -lpthread -lMS5837)
install(TARGETS app_depth DESTINATION ${project_BIN_DIR}/)
###

### metà App GPS
SET(SRC
  app_gps.cpp
)
ADD_EXECUTABLE(app_gps ${SRC})
TARGET_LINK_LIBRARIES(app_gps -lminnow_app -lyaml-cpp -lzmq -lpthread -lZOEM8)
install(TARGETS app_gps DESTINATION ${project_BIN_DIR}/)
###
