#include <iostream>
#include <sstream>
#include <signal.h>
#include <chrono>
#include <zmq.hpp>
#include "minnow_app_threaded.h"

using namespace std::chrono;

Subscriber::Subscriber(zmq::context_t* context, std::string topic) : receive_active(false), new_msg(false) {
  zmq_context = context;
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
  zmq_connect(subscriber, "tcp://127.0.0.1:5555");
  zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, subscribed_topic.c_str(), 1);
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
  sleep_ms = 1;

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
  Print("Exiting Minnow App...");
  exit(1);
}

void App::Subscribe(std::string topic) {
  std::stringstream ss;
  ss << "[M] Subscription for " << topic << " created.";
  Print(ss.str());

  Subscriber* sub = new Subscriber(zmq_context, topic);
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

void App::Run() {
  Print("Starting Minnow App!");

  Init();

  zmq_connect(*zmq_socket, "tcp://127.0.0.1:5556");

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
