#include <thread>
#include <chrono>

class A {
public:
    A();

    void startThread();
    void endThread();
private:
    void threadCall();
    std::atomic<bool> active;
};

int main() {
    A threadThing;
    threadThing.startThread();
    printf("[M] Thread Created\n");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    threadThing.endThread();
    printf("[M] Thread Killed\n");
    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}

A::A() : active(false) {

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
    while (active) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    printf("[T] Thread Ended\n");
}
