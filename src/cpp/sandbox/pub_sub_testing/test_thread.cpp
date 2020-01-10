#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>

class A {
public:
    A(std::string input);

    void startThread();
    void endThread();
private:
    void threadCall();
    std::atomic<bool> active;
    std::string test;
};

class B {
public:
    B();

    void Run();
private:
    A* a;
};

B::B() {
  a = new A("ASDF");
}

void B::Run() {
  a->startThread();
  printf("[M] Thread Created\n");
  std::this_thread::sleep_for(std::chrono::seconds(5));
  a->endThread();
  printf("[M] Thread Killed\n");
  std::this_thread::sleep_for(std::chrono::seconds(5));
}

int main() {
    B b;
    b.Run();

    return 0;
}

A::A(std::string input) : active(false) {
  test = input;
}

void A::startThread() {
    active = true;
    std::thread AThread(&A::threadCall, this);
    AThread.detach();
}

void A::endThread() {
    active = false;
}

void A::threadCall() {
    printf("[T] Thread Started\n");
    std::cout << test << std::endl;
    while (active) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    printf("[T] Thread Ended\n");
}
