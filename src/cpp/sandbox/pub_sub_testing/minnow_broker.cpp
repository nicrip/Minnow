#include <iostream>
#include <sstream>
#include "yaml-cpp/yaml.h"
#include <zmq.hpp>

int main(int argc, char* argv[]) {
  YAML::Node config;

  if(argc < 2) {
    std::cout << "Expected a YAML configuration file... Exiting." << std::endl;
    exit(0);
  }
  std::string config_file = argv[1];
  try {
		config = YAML::LoadFile(config_file);
	} catch (...) {
		std::cout << "Configuration file \"" << config_file << "\" not found... Exiting." << std::endl;
		exit(0);
	}
  std::string protocol = config["protocol"].as<std::string>();
  std::string host = config["host"].as<std::string>();
  unsigned int port;
  if(protocol == "ipc") {
    std::cout << "Using inter-process communications (ipc)." << std::endl;
  } else if(protocol == "tcp") {
    std::cout << "Using transmission control protocol (tcp)." << std::endl;
    port = config["port"].as<int>();
  } else {
    std::cout << "Unknown communications protocol... Exiting." << std::endl;
    exit(0);
  }

  zmq::context_t context(1);

  zmq::socket_t frontend(context, ZMQ_XPUB);
  std::stringstream ss;
  if(protocol == "ipc") {
    ss << "ipc://" << host << ".pub";
  } else if(protocol == "tcp") {
    ss << "tcp://" << host << ":" << port;
  }
  std::string socket = ss.str();
  std::cout << "Subscribing address: " << socket << std::endl;
  zmq_bind((void*)frontend, socket.c_str());
  ss.str("");

  zmq::socket_t backend(context, ZMQ_XSUB);
  if(protocol == "ipc") {
    ss << "ipc://" << host << ".sub";
  } else if(protocol == "tcp") {
    ss << "tcp://" << host << ":" << port+1;
  }
  socket = ss.str();
  std::cout << "Publishing address: " << socket << std::endl;
  zmq_bind((void*)backend, socket.c_str());
  ss.str("");

  std::cout << "Minnow broker started." << std::endl;

  zmq_proxy((void*)frontend, (void*)backend, nullptr);

  return 0;
}
