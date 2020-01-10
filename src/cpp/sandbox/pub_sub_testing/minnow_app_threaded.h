#include <iostream>
#include <thread>
#include <atomic>
#include <functional>
#include <zmq.hpp>

#ifndef MINNOW_APP_THREADED_HDR
#define MINNOW_APP_THREADED_HDR

class Subscriber
{
public:
  Subscriber(zmq::context_t* context, std::string topic);
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

protected:
  void ExitSignal(int s);
  void Subscribe(std::string topic);
  void CheckSubscriptions();
  void PublishString(const std::string& msg, size_t msg_size);
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

};

#endif
