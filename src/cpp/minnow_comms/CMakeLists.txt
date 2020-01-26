# FILE(GLOB SRC *.cpp)

cmake_minimum_required(VERSION 2.6)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIX -D_LINUX=1 -DCMDLINE=1")

# minnow_broker
SET(SRC
  minnow_broker.cpp
)
ADD_EXECUTABLE(minnow_broker ${SRC})
TARGET_LINK_LIBRARIES(minnow_broker -lyaml-cpp -lzmq)
install(TARGETS minnow_broker DESTINATION ${project_BIN_DIR}/)

# minnow threaded app
SET(SRC
  minnow_app_threaded.cpp
)
ADD_LIBRARY(minnow_app SHARED ${SRC})
TARGET_LINK_LIBRARIES(minnow_app -lyaml-cpp -lzmq -lpthread)
install(TARGETS minnow_app DESTINATION ${project_LIB_DIR}/)
