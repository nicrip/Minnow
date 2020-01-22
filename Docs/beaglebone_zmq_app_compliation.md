# Compiling Mezzo ZMQ+Flatbuffer Apps on Beaglebone

Compilation requires some modifications to the normal apps. 

#### In minnow_app_threaded.cpp:  
zmq::socket_t subscriber cannot be used directly. Must be casted into a void pointer:  
(void*)subscriber

std::vector<zmq::pollitem_t> p = {{subscriber, 0, ZMQ_POLLIN, 0}}; cannot be instantiated as a braced list. Must change to:  
zmq::pollitem_t p[] = {{(void*)subscriber, 0, ZMQ_POLLIN, 0}};

and zmq::poll(p.data(), 1, 1000); must change to:  
zmq::poll(&p[0], 1, 1000);

#### YAML-CPP must be built as shared libraries using:  
cmake .. -DYAML_BUILD_SHARED_LIBS=ON

#### Subclasses of the Mezzo App
Callbacks using flatbuffers must cast the message into a string as:  

std::string msg_str(msg, msg+msg_size); // required for beaglebone  
auto topic1 = topics::nav::Gettopic1(msg_str.data());  

instead of:  

auto topic1 = topics::nav::Gettopic1(msg);  

#### Finally, compilation of the Mezzo Apps
Must be performed as:  

CXX_FLAGS="-fPIE -fPIC" \
cmake ..
