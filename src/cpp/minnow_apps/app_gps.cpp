#include <iostream>
#include <sstream>
#include <zmq.hpp>
#include "flatbuffers/flatbuffers.h"
#include "../minnow_comms/minnow_app_threaded.h"
// flatbuffer topics
#include "../topics/nav_gps_generated.h"
// gps driver ZOEM8
#include "../minnow_drivers/cppZOEM8/cppZOEM8.h"

class AppGPS : public App
{
public:
  AppGPS();
  ~AppGPS();

protected:
  void Process();
  void Init();
  void CallBack(uint8_t* msg, size_t msg_size, std::string topic);

private:
  void SetMessageNavGPS();

  flatbuffers::FlatBufferBuilder* builder;
  uint8_t* msg_buf;
  unsigned int msg_size;
  ZOEM8 gps_sensor;
};

AppGPS::AppGPS() {
}

AppGPS::~AppGPS() {
}

void AppGPS::CallBack(uint8_t* msg, size_t msg_size, std::string topic) {
  std::cout << "Callback received for topic \"" << topic << "\"" << std::endl;
  std::string msg_str(msg, msg+msg_size); // required for beaglebone
}

void AppGPS::SetMessageNavGPS() {
  builder->Clear();
  auto now     = std::chrono::system_clock::now();
  auto epoch   = now.time_since_epoch();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
  double utc = milliseconds.count()/1000.0;
  double val;

  // all string variables must be initialized separately
  auto str_position_status = builder->CreateString(gps_sensor.position_status);
  auto str_mode = builder->CreateString(gps_sensor.mode);
  auto str_quality = builder->CreateString(gps_sensor.quality);

  topics::nav::gpsBuilder gps_builder(*builder);
  gps_builder.add_time(utc);
  gps_builder.add_status(str_position_status);
  gps_builder.add_mode(str_mode);
  gps_builder.add_quality(str_quality);
  gps_builder.add_longitude(gps_sensor.longitude);
  gps_builder.add_latitude(gps_sensor.latitude);
  gps_builder.add_altitude(gps_sensor.altitude);
  gps_builder.add_utc(gps_sensor.utc);
  gps_builder.add_num_satellites(gps_sensor.num_satellites);
  gps_builder.add_speed(gps_sensor.speed_over_ground);
  gps_builder.add_course(gps_sensor.course_over_ground);
  gps_builder.add_hdop(gps_sensor.horizontal_dilution);
  gps_builder.add_mag_declination(gps_sensor.magnetic_declination);

  auto gps_msg = gps_builder.Finish();
  builder->Finish(gps_msg);
  msg_buf = builder->GetBufferPointer();
  msg_size = builder->GetSize();
}

void AppGPS::Init() {
  unsigned int tick = GetConfigParameter<unsigned int>("tick");
  SetHz(tick);

  gps_sensor.init();

  builder = new flatbuffers::FlatBufferBuilder();
}

void AppGPS::Process() {
  SetMessageNavGPS();
  gps_sensor.read();
  Publish("nav.gps", msg_buf, msg_size);
}

int main(int argc, char *argv[])
{
  if(argc < 2) {
    std::cout << "Expected a YAML configuration file... Exiting." << std::endl;
    exit(0);
  }
  std::string config_file = argv[1];
  AppGPS app_gps;
  app_gps.SetName("app_gps");
  app_gps.SetConfig(config_file);
  app_gps.Run();
}
