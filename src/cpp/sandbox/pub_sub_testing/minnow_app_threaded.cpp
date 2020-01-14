#include <iostream>
#include <sstream>
#include <signal.h>
#include <chrono>
#include <zmq.hpp>
#include "minnow_app_threaded.h"

using namespace std::chrono;

Subscriber::Subscriber(zmq::context_t* context, std::string sub_address, std::string topic) : receive_active(false), new_msg(false) {
  zmq_context = context;
  address = sub_address;
  subscribed_topic = topic;
}

Subscriber::~Subscriber() {

}

void Subscriber::Start() {
  receive_active = true;
  std::thread ReceiveThread(&Subscriber::Run, this);
  ReceiveThread.detach();
}

void Subscriber::Stop() {
  receive_active = false;
}

void Subscriber::Run() {
  std::stringstream ss;
  ss << "[T] Starting subscriber thread for topic: " << subscribed_topic;
  Print(ss.str());
  ss.str("");

  zmq::socket_t subscriber(*zmq_context, ZMQ_SUB);
  zmq_connect(subscriber, address.c_str());
  zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, subscribed_topic.c_str(), subscribed_topic.size());
  std::vector<zmq::pollitem_t> p = {{subscriber, 0, ZMQ_POLLIN, 0}};
  while(receive_active) {
    zmq_poll(p.data(), 1, 1000);
    if (p[0].revents & ZMQ_POLLIN) {
      subscriber.recv(&msg, ZMQ_DONTWAIT);
      new_msg = true;
    }
  }

  ss << "[T] Stopping subscriber thread for topic: " << subscribed_topic;
  Print(ss.str());
}

////////////////////////////////////////////////////////////////////////////////

App::App() {
  zmq_context = new zmq::context_t(1);
  zmq_socket = new zmq::socket_t(*zmq_context, ZMQ_PUB);
  sleep_ms = 0;

  cpphandler = std::bind(&App::ExitSignal, this, std::placeholders::_1);
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = signal_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);
}

App::~App() {

}

void App::ExitSignal(int s) {
  std::stringstream ss;
  for(std::vector<Subscriber*>::iterator it = subscriptions.begin(); it != subscriptions.end(); ++it) {
    ss << "[M] Subscription for " << (*it)->subscribed_topic << " killed.";
    Print(ss.str());
    ss.str("");

    (*it)->Stop();
  }
  std::this_thread::sleep_for(milliseconds(1500));
  Print("Exiting Minnow App...");
  zmq_ctx_destroy(zmq_context);
  exit(1);
}

void App::Subscribe(std::string topic) {
  std::stringstream ss;
  ss << "[M] Subscription for " << topic << " created.";
  Print(ss.str());

  Subscriber* sub = new Subscriber(zmq_context, subscriber_address, topic);
  subscriptions.push_back(sub);
  sub->Start();
}

void App::CheckSubscriptions() {
  std::stringstream ss;
  for(std::vector<Subscriber*>::iterator it = subscriptions.begin(); it != subscriptions.end(); ++it) {
    if((*it)->new_msg) {
      ss << "[M] New message from " << (*it)->subscribed_topic << ".";
      Print(ss.str());
      ss.str("");
      (*it)->new_msg = false;
      std::string str;
      str.assign(static_cast<char *>((*it)->msg.data()), (*it)->msg.size());
      std::cout << "Received: " << str << std::endl;
    }
  }
}

void App::PublishString(const std::string& msg, size_t msg_size) {
  int rc = zmq_send(*zmq_socket, msg.data(), msg_size, 0);
}

void App::Publish(std::string topic, uint8_t* msg, size_t msg_size) {
  topic.append("_");
  unsigned int total_len = msg_size + topic.size();
  uint8_t *buffer= new uint8_t[total_len];
  memcpy(buffer, topic.data(), sizeof(topic));
  memcpy(buffer+sizeof(topic), msg, sizeof(msg));
  int rc = zmq_send(*zmq_socket, buffer, (size_t)total_len, 0);
}

void App::SetConfig(std::string config_file) {
  try {
		config = YAML::LoadFile(config_file);
	} catch (...) {
		std::cout << "[C] Configuration file \"" << config_file << "\" not found... Exiting." << std::endl;
		exit(0);
	}
  try {
    protocol = config["protocol"].as<std::string>();
    host = config["host"].as<std::string>();
    if(protocol == "ipc") {
      std::cout << "[C] Using inter-process communications (ipc)." << std::endl;
    } else if(protocol == "tcp") {
      std::cout << "[C] Using transmission control protocol (tcp)." << std::endl;
      port = config["port"].as<int>();
    } else {
      std::cout << "[C] Unknown communications protocol... Exiting." << std::endl;
      exit(0);
    }
  } catch (...) {
    std::cout << "[C] Configuration file must define \"protocol\", \"host\" and, if required, \"port\"... Exiting." << std::endl;
    exit(0);
  }
}

void App::Run() {
  std::stringstream ss;
  ss << "[M] Starting Minnow App \"" << name << "\"!";
  Print(ss.str());
  ss.str("");

  if(protocol == "ipc") {
    ss << "ipc://" << host << ".sub";
  } else if(protocol == "tcp") {
    ss << "tcp://" << host << ":" << port+1;
  }
  std::string socket = ss.str();
  std::cout << "[C] Publishing address: " << socket << std::endl;
  zmq_connect(*zmq_socket, socket.c_str());
  ss.str("");
  if(protocol == "ipc") {
    ss << "ipc://" << host << ".pub";
  } else if(protocol == "tcp") {
    ss << "tcp://" << host << ":" << port;
  }
  subscriber_address = ss.str();
  std::cout << "[C] Subscribing address: " << subscriber_address << std::endl;

  Init();

  while(true) {
    auto start = high_resolution_clock::now();

    CheckSubscriptions();
    Process();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    int rem_usecs = sleep_ms*1000 - duration.count();
    if(rem_usecs > 0) {
      std::this_thread::sleep_for(microseconds(rem_usecs));
    }
  }
}
