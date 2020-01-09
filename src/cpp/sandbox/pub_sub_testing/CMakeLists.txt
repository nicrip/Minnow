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
TARGET_LINK_LIBRARIES(minnow_broker -lzmq)
install(TARGETS minnow_broker DESTINATION ${project_BIN_DIR}/)

# minnow_threaded_app
SET(SRC
  minnow_app_threaded.cpp
)
ADD_EXECUTABLE(minnow_threaded_app ${SRC})
TARGET_LINK_LIBRARIES(minnow_threaded_app -lzmq -lpthread)
install(TARGETS minnow_broker DESTINATION ${project_BIN_DIR}/)

# minnow_test
SET(SRC
  minnow_app_threaded_test.cpp
)
ADD_EXECUTABLE(minnow_test ${SRC})
TARGET_LINK_LIBRARIES(minnow_test -lzmq -lpthread)
install(TARGETS minnow_broker DESTINATION ${project_BIN_DIR}/)
