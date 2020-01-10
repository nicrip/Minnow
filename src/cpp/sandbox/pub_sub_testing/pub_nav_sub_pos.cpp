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
  count = 0;
  Subscribe("POS_");
  Subscribe("PRESS_");
}

void SampleApp1::Process() {
  std::stringstream ss;
  count = count + 1;
  ss << "NAV_" << count;
  std::string msg = ss.str();
  PublishString(msg, msg.length());
  ss.str("");
}

int main(int argc, char *argv[])
{
  SampleApp1 sample_app1;
  sample_app1.Run();
}
