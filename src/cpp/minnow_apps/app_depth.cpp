#include <iostream>
#include <sstream>
#include <zmq.hpp>
#include "flatbuffers/flatbuffers.h"
#include "../minnow_comms/minnow_app_threaded.h"
// flatbuffer topics
#include "../topics/nav_depth_generated.h"
// depth driver MS5837
#include "../minnow_drivers/cppMS5837/cppMS5837.h"

class AppDepth : public App
{
public:
  AppDepth();
  ~AppDepth();

protected:
  void Process();
  void Init();
  void CallBack(uint8_t* msg, size_t msg_size, std::string topic);

private:
  void SetMessageNavDepth();

  flatbuffers::FlatBufferBuilder* builder;
  uint8_t* msg_buf;
  unsigned int msg_size;
  MS5837 depth_sensor;
};

AppDepth::AppDepth() {
}

AppDepth::~AppDepth() {
}

void AppDepth::CallBack(uint8_t* msg, size_t msg_size, std::string topic) {
  std::cout << "Callback received for topic \"" << topic << "\"" << std::endl;
  std::string msg_str(msg, msg+msg_size); // required for beaglebone
}

void AppDepth::SetMessageNavDepth() {
  builder->Clear();
  auto now     = std::chrono::system_clock::now();
  auto epoch   = now.time_since_epoch();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
  double utc = milliseconds.count()/1000.0;
  double val;

  topics::nav::depthBuilder depth_builder(*builder);
  depth_builder.add_time(utc);
  val = depth_sensor.temperature();
  depth_builder.add_temperature(val);
  val = depth_sensor.pressure();
  depth_builder.add_pressure(val);
  val = depth_sensor.depth();
  depth_builder.add_depth_fluid(val);

  auto depth_msg = depth_builder.Finish();
  builder->Finish(depth_msg);
  msg_buf = builder->GetBufferPointer();
  msg_size = builder->GetSize();
}

void AppDepth::Init() {
  unsigned int tick = GetConfigParameter<unsigned int>("tick");
  SetHz(tick);

  float fluid_density = GetConfigParameter<float>("fluid_density");
  unsigned int oversampling = GetConfigParameter<unsigned int>("oversampling");
  depth_sensor.init();
	depth_sensor.setModel(MS5837::MS5837_30BA);
  depth_sensor.setFluidDensity(fluid_density);
  depth_sensor.setOverSampling(oversampling);

  builder = new flatbuffers::FlatBufferBuilder();
}

void AppDepth::Process() {
  SetMessageNavDepth();
  depth_sensor.read();
  Publish("nav.depth", msg_buf, msg_size);
}

int main(int argc, char *argv[])
{
  if(argc < 2) {
    std::cout << "Expected a YAML configuration file... Exiting." << std::endl;
    exit(0);
  }
  std::string config_file = argv[1];
  AppDepth app_depth;
  app_depth.SetName("app_depth");
  app_depth.SetConfig(config_file);
  app_depth.Run();
}
