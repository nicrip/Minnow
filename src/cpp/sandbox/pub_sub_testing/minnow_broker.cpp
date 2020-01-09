#include <iostream>
#include <zmq.hpp>

int main(int argc, char* argv[]) {
    zmq::context_t context(1);

    zmq::socket_t frontend(context, ZMQ_XSUB);
    frontend.bind("tcp://*:5555");

    zmq::socket_t backend(context, ZMQ_XPUB);
    backend.bind("tcp://*:5556");

    std::cout << "Minnow broker started." << std::endl;

    zmq::proxy(frontend, backend, nullptr);

    return 0;
}
