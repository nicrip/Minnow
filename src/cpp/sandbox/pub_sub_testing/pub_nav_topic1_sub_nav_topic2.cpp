#include <iostream>
#include <sstream>
#include <zmq.hpp>
#include "flatbuffers/flatbuffers.h"
#include "minnow_app_threaded.h"
// flatbuffer topics
#include "topics/nav_topic1_generated.h"
#include "topics/nav_topic2_generated.h"

class SampleApp1 : public App
{
public:
  SampleApp1();
  ~SampleApp1();

protected:
  void Process();
  void Init();

private:
  flatbuffers::FlatBufferBuilder* builder;
  unsigned int count;
};

SampleApp1::SampleApp1() {

}

SampleApp1::~SampleApp1() {

}

void SampleApp1::Init() {
  unsigned int tick = GetConfigParameter<unsigned int>("tick");
  SetTick(tick);
  count = 0;
  builder = new flatbuffers::FlatBufferBuilder();

  Subscribe("nav.topics");
}

void SampleApp1::Process() {
  builder->Clear();

  auto now     = std::chrono::system_clock::now();
  auto epoch   = now.time_since_epoch();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
  double utc = milliseconds.count()/1000.0;

  std::stringstream ss;
  count = count + 1;
  ss << "topic1_" << count;
  std::string msg = ss.str();
  auto test_string = builder->CreateString(msg);
  ss.str("");

  topics::nav::topic1Builder topic1_builder(*builder);
  topic1_builder.add_time(utc);
  topic1_builder.add_test(test_string);
  auto topic1_msg = topic1_builder.Finish();
  builder->Finish(topic1_msg);
  uint8_t* topic1_buf = builder->GetBufferPointer();
  int topic1_size = builder->GetSize();

  Publish("nav.topic1", topic1_buf, topic1_size);
}

int main(int argc, char *argv[])
{
  if(argc < 2) {
    std::cout << "Expected a YAML configuration file... Exiting." << std::endl;
    exit(0);
  }
  std::string config_file = argv[1];
  SampleApp1 sample_app1;
  sample_app1.SetAppName("test_app_topic1");
  sample_app1.SetConfig(config_file);
  sample_app1.Run();
}
