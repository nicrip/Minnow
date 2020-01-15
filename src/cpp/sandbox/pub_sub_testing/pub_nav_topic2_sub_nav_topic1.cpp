#include <iostream>
#include <sstream>
#include <zmq.hpp>
#include "flatbuffers/flatbuffers.h"
#include "minnow_app_threaded.h"
// flatbuffer topics
#include "topics/nav_topic1_generated.h"
#include "topics/nav_topic2_generated.h"

class SampleApp2 : public App
{
public:
  SampleApp2();
  ~SampleApp2();

protected:
  void Process();
  void Init();
  void CallBack(uint8_t* msg, size_t msg_size);

private:
  void SetMessageTopic2();

  flatbuffers::FlatBufferBuilder* builder;
  uint8_t* msg_buf;
  unsigned int msg_size;
  unsigned int count;
};

SampleApp2::SampleApp2() {

}

SampleApp2::~SampleApp2() {

}

void SampleApp2::CallBack(uint8_t* msg, size_t msg_size) {
  std::cout << "Callback for nav.topic1" << std::endl;
  auto topic1 = topics::nav::Gettopic1(msg);
  auto time = topic1->time();
  auto val = topic1->test()->c_str();
  std::cout << std::fixed << time << " " << val << std::endl;
}

void SampleApp2::SetMessageTopic2() {
  builder->Clear();
  auto now     = std::chrono::system_clock::now();
  auto epoch   = now.time_since_epoch();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
  double utc = milliseconds.count()/1000.0;
  std::stringstream ss;
  count = count + 1;
  ss << "topic2 message: " << count;
  std::string msg = ss.str();
  auto test_string = builder->CreateString(msg);
  topics::nav::topic2Builder topic2_builder(*builder);
  topic2_builder.add_time(utc);
  topic2_builder.add_test(test_string);
  auto topic2_msg = topic2_builder.Finish();
  builder->Finish(topic2_msg);
  msg_buf = builder->GetBufferPointer();
  msg_size = builder->GetSize();
}

void SampleApp2::Init() {
  unsigned int tick = GetConfigParameter<unsigned int>("tick");
  SetHz(tick);
  count = 0;
  builder = new flatbuffers::FlatBufferBuilder();
  Subscribe("nav.topic1", [this](uint8_t* msg, size_t msg_size){CallBack(msg, msg_size);});
}

void SampleApp2::Process() {
  SetMessageTopic2();
  Publish("nav.topic2", msg_buf, msg_size);
}

int main(int argc, char *argv[])
{
  if(argc < 2) {
    std::cout << "Expected a YAML configuration file... Exiting." << std::endl;
    exit(0);
  }
  std::string config_file = argv[1];
  SampleApp2 sample_app2;
  sample_app2.SetName("test_app_topic2");
  sample_app2.SetConfig(config_file);
  sample_app2.Run();
}
