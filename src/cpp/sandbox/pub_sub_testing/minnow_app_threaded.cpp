#include <iostream>
#include <sstream>
#include <signal.h>
#include <zmq.hpp>
#include "minnow_app_threaded.h"

using namespace std::chrono;

Subscriber::Subscriber(App* app, std::string address, std::string topic, std::function<void(uint8_t* msg, size_t msg_size, std::string topic)> callback) : receive_active_(false), new_msg_(false) {
  app_ = app;
  zmq_context_ = app_->zmq_context_;
  address_ = address;
  topic_ = topic;
  callback_ = callback;
}

Subscriber::~Subscriber() {
  receive_thread_.join();
}

void Subscriber::Start() {
  receive_active_ = true;
  receive_thread_ = std::thread(&Subscriber::Run, this);
}

void Subscriber::Stop() {
  receive_active_ = false;
}

void Subscriber::Run() {
  std::stringstream ss;
  ss << "[T] Starting subscriber thread for topic: " << topic_;
  Print(ss.str());
  ss.str("");

  // variables for message matching
  std::string msg_str;
  size_t msg_size;
  bool subscribed;

  zmq::socket_t subscriber(*zmq_context_, ZMQ_SUB);
  zmq_connect(subscriber, address_.c_str());
  zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, topic_.c_str(), topic_.size());
  std::vector<zmq::pollitem_t> p = {{subscriber, 0, ZMQ_POLLIN, 0}};
  while(receive_active_) {
    zmq_poll(p.data(), 1, 1000);
    if (p[0].revents & ZMQ_POLLIN) {
      mutex_.lock();
      subscriber.recv(&msg_, ZMQ_DONTWAIT);
      // copy message into local variable for checking
      msg_str.assign(static_cast<char *>(msg_.data()), msg_.size());
      msg_size = msg_.size();
      mutex_.unlock();

      // check for message match - if not a match, spin up a new subscriber for the new message
      size_t msg_start = msg_str.find_first_of("_");
      std::string topic = msg_str.substr(0,msg_start);
      if(topic == topic_) {
        new_msg_ = true;
      } else {
        new_msg_ = false;
        subscribed = app_->CheckSubscriptionToTopic(topic);
        if(!subscribed) {
          app_->Subscribe(topic, callback_);
        }
      }
    }
  }

  ss << "[T] Stopping subscriber thread for topic: " << topic_;
  Print(ss.str());
}

////////////////////////////////////////////////////////////////////////////////

App::App() {
  zmq_context_ = new zmq::context_t(1);
  zmq_socket_ = new zmq::socket_t(*zmq_context_, ZMQ_PUB);
  sleep_us_ = 10000;  // 100Hz default

  cpphandler = std::bind(&App::ExitSignal, this, std::placeholders::_1);
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = signal_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);
}

App::~App() {
  for(std::vector<Subscriber*>::iterator it = subscriptions_.begin(); it != subscriptions_.end(); ++it) {
    delete *it;
  }
  delete zmq_socket_;
  delete zmq_context_;
}

void App::ExitSignal(int s) {
  std::stringstream ss;
  for(std::vector<Subscriber*>::iterator it = subscriptions_.begin(); it != subscriptions_.end(); ++it) {
    ss << "[M] Subscription for " << (*it)->topic_ << " killed.";
    Print(ss.str());
    ss.str("");

    (*it)->Stop();
  }
  std::this_thread::sleep_for(milliseconds(1500));
  Print("Exiting Minnow App...");
  zmq_ctx_destroy(zmq_context_);
  exit(1);
}

void App::Subscribe(std::string topic, std::function<void(uint8_t* msg, size_t msg_size, std::string topic)> callback) {
  std::stringstream ss;
  ss << "[M] Subscription for " << topic << " created.";
  Print(ss.str());

  Subscriber* sub = new Subscriber(this, subscriber_address_, topic, callback);
  mutex_.lock();
  subscriptions_.push_back(sub);
  subscription_topics_.push_back(topic);
  mutex_.unlock();
  sub->Start();
}

bool App::CheckSubscriptionToTopic(std::string topic) {
  bool subscribed = false;
  for(std::vector<std::string>::iterator it = subscription_topics_.begin(); it != subscription_topics_.end(); ++it) {
    if(topic == (*it)) subscribed = true;
  }
  return subscribed;
}

void App::CheckSubscriptions() {
  std::stringstream ss;
  mutex_.lock();
  for(std::vector<Subscriber*>::iterator it = subscriptions_.begin(); it != subscriptions_.end(); ++it) {
    if((*it)->new_msg_) {
      ss << "[M] New message from " << (*it)->topic_ << ".";
      Print(ss.str());
      ss.str("");
      (*it)->new_msg_ = false;
      std::string msg_str;
      size_t msg_size;
      (*it)->mutex_.lock();
      msg_str.assign(static_cast<char *>((*it)->msg_.data()), (*it)->msg_.size());
      msg_size = (*it)->msg_.size();
      (*it)->mutex_.unlock();
      size_t msg_start = msg_str.find_first_of("_");
      (*it)->callback_((uint8_t*)msg_str.data()+(msg_start+1), msg_size-(msg_start+1), msg_str.substr(0,msg_start));
    }
  }
  mutex_.unlock();
}

void App::PublishString(std::string topic, std::string msg, size_t msg_size) {
  Publish(topic, (uint8_t*)msg.data(), msg.length());
}

void App::Publish(std::string topic, uint8_t* msg, size_t msg_size) {
  topic.append("_");
  unsigned int total_len = msg_size + topic.length();
  uint8_t *buffer= new uint8_t[total_len];
  memcpy(buffer, topic.data(), topic.length());
  memcpy(buffer+topic.length(), msg, msg_size);
  int rc = zmq_send(*zmq_socket_, buffer, (size_t)total_len, 0);
}

void App::SetConfig(std::string config_file) {
  try {
		config_ = YAML::LoadFile(config_file);
	} catch (...) {
		std::cout << "[C] Configuration file \"" << config_file << "\" not found... Exiting." << std::endl;
		exit(0);
	}
  try {
    protocol_ = config_["protocol"].as<std::string>();
    host_ = config_["host"].as<std::string>();
    if(protocol_ == "ipc") {
      std::cout << "[C] Using inter-process communications (ipc)." << std::endl;
    } else if(protocol_ == "tcp") {
      std::cout << "[C] Using transmission control protocol (tcp)." << std::endl;
      port_ = config_["port"].as<int>();
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
  ss << "[M] Starting Minnow App \"" << name_ << "\"!";
  Print(ss.str());
  ss.str("");

  if(protocol_ == "ipc") {
    ss << "ipc://" << host_ << ".sub";
  } else if(protocol_ == "tcp") {
    ss << "tcp://" << host_ << ":" << port_+1;
  }
  std::string socket = ss.str();
  std::cout << "[C] Publishing address: " << socket << std::endl;
  zmq_connect(*zmq_socket_, socket.c_str());
  ss.str("");
  if(protocol_ == "ipc") {
    ss << "ipc://" << host_ << ".pub";
  } else if(protocol_ == "tcp") {
    ss << "tcp://" << host_ << ":" << port_;
  }
  subscriber_address_ = ss.str();
  std::cout << "[C] Subscribing address: " << subscriber_address_ << std::endl;

  Init();

  while(true) {
    if(sleep_us_ != 0) loop_start_ = high_resolution_clock::now();

    CheckSubscriptions();
    Process();

    if(sleep_us_ != 0) {
      loop_end_ = high_resolution_clock::now();
      loop_us_ = duration_cast<microseconds>(loop_end_ - loop_start_);
      int rem_usecs = sleep_us_ - loop_us_.count();
      if(rem_usecs > 0) {
        std::this_thread::sleep_for(microseconds(rem_usecs));
      }
    }
  }
}
