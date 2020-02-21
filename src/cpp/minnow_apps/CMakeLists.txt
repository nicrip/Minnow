# FILE(GLOB SRC *.cpp)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIX -D_LINUX=1 -DCMDLINE=1")

### metà App Depth
SET(SRC
  app_depth.cpp
)
ADD_EXECUTABLE(app_depth ${SRC})
TARGET_LINK_LIBRARIES(app_depth minnow_app -lyaml-cpp -lzmq -lpthread MS5837)
install(TARGETS app_depth DESTINATION ${project_BIN_DIR}/)
###

### metà App GPS
SET(SRC
  app_gps.cpp
)
ADD_EXECUTABLE(app_gps ${SRC})
TARGET_LINK_LIBRARIES(app_gps minnow_app -lyaml-cpp -lzmq -lpthread ZOEM8)
install(TARGETS app_gps DESTINATION ${project_BIN_DIR}/)
###

### metà App IMU
SET(SRC
  app_imu.cpp
)
ADD_EXECUTABLE(app_imu ${SRC})
TARGET_LINK_LIBRARIES(app_imu minnow_app -lyaml-cpp -lzmq -lpthread USFS)
install(TARGETS app_imu DESTINATION ${project_BIN_DIR}/)
###
