#include <iostream>
#include <thread>
#include <atomic>
#include <zmq.hpp>

#ifndef MINNOW_APP_THREADED_HDR
#define MINNOW_APP_THREADED_HDR

class Subscriber
{
  public:
    Subscriber();
    ~Subscriber();
    void Start();
    void Stop();

  protected:
    void Run();
    template <typename T>
    void Print(T a) {
      std::cout << a << std::endl;
    }

    std::atomic<bool> shutdown_flag;
    // std::string subscribed_topic;
};

class App
{
  public:
    App();
    ~App();
    void Run();

  protected:
    void ExitSignal(int s);
    void Process();
    void Publish(zmq::message_t msg, size_t msg_size);
    void PublishString(const std::string& msg, size_t msg_size);
    void Subscribe(std::string topic, void (*f)(zmq::message_t));
    void CheckSubscriptions();
    template <typename T>
    void Print(T a) {
      std::cout << a << std::endl;
    }

    zmq::context_t zmq_context;
    zmq::socket_t socket;
    std::vector<Subscriber*> subscriptions;

  private:
    int count;
};

#endif
