#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <chrono>
#include "yaml-cpp/yaml.h"
#include <zmq.hpp>

#ifndef MINNOW_APP_THREADED_HDR
#define MINNOW_APP_THREADED_HDR

class Subscriber
{
public:
  Subscriber(zmq::context_t* context, std::string address, std::string topic, std::function<void(uint8_t* msg, size_t msg_size)> callback);
  ~Subscriber();
  void Start();
  void Stop();

  std::string topic_;
  std::atomic<bool> new_msg_;
  std::mutex mutex_;
  zmq::message_t msg_;
  std::function<void(uint8_t* msg, size_t msg_size)> callback_;

private:
  void Run();
  template <typename T>
  void Print(T a) {
    std::cout << a << std::endl;
  }

  std::string address_;
  zmq::context_t* zmq_context_;
  std::atomic<bool> receive_active_;
};

////////////////////////////////////////////////////////////////////////////////

std::function<void(int)> cpphandler = NULL;
extern "C" {
    void signal_handler(int i);
}
void signal_handler(int i){
    cpphandler(i);
    return;
}

////////////////////////////////////////////////////////////////////////////////

class App
{
public:
  App();
  ~App();
  void Run();
  void SetName(std::string name) {
    name_ = name;
  }
  void SetConfig(std::string config_file);

protected:
  void ExitSignal(int s);
  void Subscribe(std::string topic, std::function<void(uint8_t* msg, size_t msg_size)> callback);
  void CheckSubscriptions();
  void PublishString(std::string topic, std::string msg, size_t msg_size);
  void Publish(std::string topic, uint8_t* msg, size_t msg_size);
  template <typename T>
  T GetConfigParameter(std::string config_param) {
    try {
      T value = config_[name_][config_param].as<T>();
      return value;
    } catch (...) {
      std::cout << "[C] Configuration parameter \"" << config_param << "\" for Minnow App \"" << name_ << "\" not found... Exiting." << std::endl;
      exit(0);
    }
  }
  void SetHz(unsigned int tick_hz) {
    if(tick_hz < 0) {
      std::cout << "[C] Configuration parameter \"tick\" for Minnow App \"" << name_ << "\" must be positive or zero... Exiting." << std::endl;
      exit(0);
    } else if(tick_hz == 0) {
      sleep_us_ = 0;
    } else {
      sleep_us_ = 1e6/tick_hz;
    }
  }
  virtual void Init() = 0;    // must be implemented by subclasses
  virtual void Process() = 0; // must be implemented by subclasses

  zmq::context_t* zmq_context_;
  zmq::socket_t* zmq_socket_;
  std::vector<Subscriber*> subscriptions_;
  unsigned int sleep_us_;

private:
  template <typename T>
  void Print(T a) {
    std::cout << a << std::endl;
  }

  std::string name_;
  YAML::Node config_;
  std::string protocol_;
  std::string host_;
  unsigned int port_;
  std::string subscriber_address_;
  std::chrono::high_resolution_clock::time_point loop_start_;
  std::chrono::high_resolution_clock::time_point loop_end_;
  std::chrono::microseconds loop_us_;
};

#endif
