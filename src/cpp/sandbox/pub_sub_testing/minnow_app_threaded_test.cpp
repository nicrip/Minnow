#include <iostream>
#include <sstream>
#include <zmq.hpp>
#include "minnow_app_threaded.h"

#include <chrono>

Subscriber::Subscriber() : shutdown_flag(false) {

  // subscribed_topic = topic;
}

Subscriber::~Subscriber() {

}

void Subscriber::Start() {
  std::thread SubscriberThread(&Subscriber::Run, this);
  SubscriberThread.detach();
}

void Subscriber::Stop() {
  shutdown_flag = true;
}

void Subscriber::Run() {
  std::stringstream ss;
  ss << "Starting subscriber for topic: ";
  Print(ss.str());
  ss.str("");

  zmq::context_t zmq_context(1);
  zmq::socket_t subscriber(zmq_context, ZMQ_SUB);
  zmq_connect((void*)subscriber, "tcp://127.0.0.1:5555");
  zmq_setsockopt((void*)subscriber, ZMQ_SUBSCRIBE, "PRESS_", 1);
  //std::vector<zmq::pollitem_t> p = {{subscriber, 0, ZMQ_POLLIN, 0}};
  zmq::pollitem_t p[] = {{(void*)subscriber, 0, ZMQ_POLLIN, 0}};
  while(!shutdown_flag) {
    zmq::message_t rx_msg;
    //zmq::poll(p.data(), 1, 1000);
    zmq::poll(&p[0], 1, 1000);
    if (p[0].revents & ZMQ_POLLIN) {
      // received something on the first (only) socket
      subscriber.recv(&rx_msg);
      std::string rx_str;
      rx_str.assign(static_cast<char *>(rx_msg.data()), rx_msg.size());
      std::cout << "Received: " << rx_str << std::endl;
    }
  }
  ss << "Ending subscriber for topic: ";
  Print(ss.str());
}

App::App() : zmq_context(1), socket(zmq_context, ZMQ_PUB), count(0) {
  Subscriber test;
  test.Start();
  subscriptions.push_back(&test);
}

App::~App() {

}

void App::ExitSignal(int s) {

}

void App::Process() {
  std::stringstream ss;
  count = count + 1;
  ss << "TEST_" << count;
  std::string msg = ss.str();
  PublishString(msg, msg.length());
  ss.str("");
}

void App::Publish(zmq::message_t msg, size_t msg_size) {
  int rc = zmq_send((void*)socket, &msg, msg_size, 0);
}

void App::PublishString(const std::string& msg, size_t msg_size) {
  int rc = zmq_send((void*)socket, msg.data(), msg_size, 0);
}

void App::Subscribe(std::string topic, void (*f)(zmq::message_t)) {

}

void App::CheckSubscriptions() {

}

void App::Run() {
  Print("Starting publisher");
  zmq_connect((void*)socket, "tcp://127.0.0.1:5556");
  while(true) {
    CheckSubscriptions();
    Process();
  }
}

int main(int argc, char *argv[])
{
  App ThreadedApp;
  ThreadedApp.Run();
}
