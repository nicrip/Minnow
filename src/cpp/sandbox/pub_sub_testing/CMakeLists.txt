# FILE(GLOB SRC *.cpp)

cmake_minimum_required(VERSION 2.6)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIX -D_LINUX=1 -DCMDLINE=1")

get_filename_component(project_BIN_DIR bin ABSOLUTE)

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

# Minnow example app 1
SET(SRC
  pub_nav_sub_pos.cpp
)
ADD_EXECUTABLE(pub_nav_sub_pos ${SRC})
TARGET_LINK_LIBRARIES(pub_nav_sub_pos minnow_app -lyaml-cpp -lzmq -lpthread )
install(TARGETS pub_nav_sub_pos DESTINATION ${project_BIN_DIR}/)

# Minnow example app 2
SET(SRC
  pub_pos_sub_nav.cpp
)
ADD_EXECUTABLE(pub_pos_sub_nav ${SRC})
TARGET_LINK_LIBRARIES(pub_pos_sub_nav minnow_app -lyaml-cpp -lzmq -lpthread )
install(TARGETS pub_pos_sub_nav DESTINATION ${project_BIN_DIR}/)

# Minnow example app with flatbuffers 1
SET(SRC
  pub_nav_topic1_sub_nav_topic2.cpp
)
ADD_EXECUTABLE(pub_nav_topic1_sub_nav_topic2 ${SRC})
TARGET_LINK_LIBRARIES(pub_nav_topic1_sub_nav_topic2 minnow_app -lyaml-cpp -lzmq -lpthread )
install(TARGETS pub_nav_topic1_sub_nav_topic2 DESTINATION ${project_BIN_DIR}/)
