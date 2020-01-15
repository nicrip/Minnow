#include <iostream>
#include <sstream>
#include <zmq.hpp>
#include "minnow_app_threaded.h"

class SampleApp1 : public App
{
public:
  SampleApp1();
  ~SampleApp1();

protected:
  void Process();
  void Init();
  void CallBack(uint8_t* msg, size_t msg_size);

private:
  unsigned int count;
};

SampleApp1::SampleApp1() {

}

SampleApp1::~SampleApp1() {

}

void SampleApp1::CallBack(uint8_t* msg, size_t msg_size) {
  std::string test(msg, msg+msg_size);
  std::cout << "Callback received: " << test << std::endl;
}

void SampleApp1::Init() {
  unsigned int tick = GetConfigParameter<unsigned int>("tick");
  SetHz(tick);
  count = 0;
  Subscribe("NAV", [this](uint8_t* msg, size_t msg_size){CallBack(msg, msg_size);});
}

void SampleApp1::Process() {
  std::stringstream ss;
  count = count + 1;
  ss << "position message: " << count;
  std::string msg = ss.str();
  PublishString("POS", msg, msg.length());
  ss.str("");

  ss << "pressure message: " << count;
  msg = ss.str();
  PublishString("PRESS", msg, msg.length());
}

int main(int argc, char *argv[])
{
  if(argc < 2) {
    std::cout << "Expected a YAML configuration file... Exiting." << std::endl;
    exit(0);
  }
  std::string config_file = argv[1];
  SampleApp1 sample_app1;
  sample_app1.SetName("test_app2");
  sample_app1.SetConfig(config_file);
  sample_app1.Run();
}
