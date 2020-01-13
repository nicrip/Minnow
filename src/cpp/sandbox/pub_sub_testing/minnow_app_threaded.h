#include <iostream>
#include <thread>
#include <atomic>
#include <functional>
#include "yaml-cpp/yaml.h"
#include <zmq.hpp>

#ifndef MINNOW_APP_THREADED_HDR
#define MINNOW_APP_THREADED_HDR

class Subscriber
{
public:
  Subscriber(zmq::context_t* context, std::string sub_address, std::string topic);
  ~Subscriber();
  void Start();
  void Stop();

  std::string subscribed_topic;
  std::atomic<bool> new_msg;
  zmq::message_t msg;

private:
  void Run();
  template <typename T>
  void Print(T a) {
    std::cout << a << std::endl;
  }

  std::string address;
  zmq::context_t* zmq_context;
  std::atomic<bool> receive_active;
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
  void SetAppName(std::string app_name) {
    name = app_name;
  }
  void SetConfig(std::string config_file);

protected:
  void ExitSignal(int s);
  void Subscribe(std::string topic);
  void CheckSubscriptions();
  void PublishString(const std::string& msg, size_t msg_size);
  void Publish(std::string topic ,uint8_t* msg, size_t msg_size);
  template <typename T>
  T GetConfigParameter(std::string config_param) {
    try {
      T value = config[name][config_param].as<T>();
      return value;
    } catch (...) {
      std::cout << "[C] Configuration parameter \"" << config_param << "\" for Minnow App \"" << name << "\" not found... Exiting." << std::endl;
      exit(0);
    }
  }
  void SetTick(unsigned int tick_ms) {
    sleep_ms = tick_ms;
  }
  virtual void Init() = 0;    // must be implemented by subclasses
  virtual void Process() = 0; // must be implemented by subclasses

  zmq::context_t* zmq_context;
  zmq::socket_t* zmq_socket;
  std::vector<Subscriber*> subscriptions;
  unsigned int sleep_ms;

private:
  template <typename T>
  void Print(T a) {
    std::cout << a << std::endl;
  }

  std::string name;
  YAML::Node config;
  std::string protocol;
  std::string host;
  unsigned int port;
  std::string subscriber_address;
};

#endif
