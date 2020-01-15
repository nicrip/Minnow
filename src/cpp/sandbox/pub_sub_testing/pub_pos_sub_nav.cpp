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

private:
  unsigned int count;
};

SampleApp1::SampleApp1() {

}

SampleApp1::~SampleApp1() {

}

void SampleApp1::Init() {
  unsigned int tick = GetConfigParameter<unsigned int>("tick");
  SetHz(tick);
  count = 0;
  Subscribe("NAV_");
}

void SampleApp1::Process() {
  std::stringstream ss;
  count = count + 1;
  ss << "POS_" << count;
  std::string msg = ss.str();
  PublishString(msg, msg.length());
  ss.str("");

  ss << "PRESS_" << count;
  msg = ss.str();
  PublishString(msg, msg.length());
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
