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
  void CallBack(uint8_t* msg, size_t msg_size, std::string topic);

private:
  void SetMessageTopic1();

  flatbuffers::FlatBufferBuilder* builder;
  uint8_t* msg_buf;
  unsigned int msg_size;
  unsigned int count;
};

SampleApp1::SampleApp1() {

}

SampleApp1::~SampleApp1() {

}

void SampleApp1::CallBack(uint8_t* msg, size_t msg_size, std::string topic) {
  std::cout << "Callback received for topic \"" << topic << "\"" << std::endl;
  auto topic2 = topics::nav::Gettopic2(msg);
  auto time = topic2->time();
  auto val = topic2->test()->c_str();
  std::cout << std::fixed << time << " " << val << std::endl;
}

void SampleApp1::SetMessageTopic1() {
  builder->Clear();
  auto now     = std::chrono::system_clock::now();
  auto epoch   = now.time_since_epoch();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
  double utc = milliseconds.count()/1000.0;
  std::stringstream ss;
  count = count + 1;
  ss << "topic1 message: " << count;
  std::string msg = ss.str();
  auto test_string = builder->CreateString(msg);
  topics::nav::topic1Builder topic1_builder(*builder);
  topic1_builder.add_time(utc);
  topic1_builder.add_test(test_string);
  auto topic1_msg = topic1_builder.Finish();
  builder->Finish(topic1_msg);
  msg_buf = builder->GetBufferPointer();
  msg_size = builder->GetSize();
}

void SampleApp1::Init() {
  unsigned int tick = GetConfigParameter<unsigned int>("tick");
  SetHz(tick);
  count = 0;
  builder = new flatbuffers::FlatBufferBuilder();
  Subscribe("nav.topic2", [this](uint8_t* msg, size_t msg_size, std::string topic){CallBack(msg, msg_size, topic);});
}

void SampleApp1::Process() {
  SetMessageTopic1();
  Publish("nav.topic1", msg_buf, msg_size);
  PublishString("nav.fakemsg", "FAKE MESSAGE", 12);
}

int main(int argc, char *argv[])
{
  if(argc < 2) {
    std::cout << "Expected a YAML configuration file... Exiting." << std::endl;
    exit(0);
  }
  std::string config_file = argv[1];
  SampleApp1 sample_app1;
  sample_app1.SetName("test_app_topic1");
  sample_app1.SetConfig(config_file);
  sample_app1.Run();
}
