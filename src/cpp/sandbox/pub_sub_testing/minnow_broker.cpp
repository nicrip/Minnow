#include <iostream>
#include <zmq.hpp>

int main(int argc, char* argv[]) {
    zmq::context_t context(1);

    zmq::socket_t frontend(context, ZMQ_XPUB);
    zmq_bind((void*)frontend, "tcp://*:5555");

    zmq::socket_t backend(context, ZMQ_XSUB);
    zmq_bind((void*)backend, "tcp://*:5556");

    std::cout << "Minnow broker started." << std::endl;

    zmq_proxy((void*)frontend, (void*)backend, nullptr);

    return 0;
}
