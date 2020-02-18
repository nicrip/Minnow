#include <iostream>
#include <sstream>
#include <zmq.hpp>
#include "flatbuffers/flatbuffers.h"
#include "../minnow_comms/minnow_app_threaded.h"
// flatbuffer topics
#include "../topics/nav_gps_generated.h"
#include "../topics/nav_imu_generated.h"
// imu driver USFS
#include "../minnow_drivers/cppUSFS/cppUSFS.h"

class AppIMU : public App
{
public:
  AppIMU();
  ~AppIMU();

protected:
  void Process();
  void Init();
  void CallBack(uint8_t* msg, size_t msg_size, std::string topic);

private:
  void SetMessageNavIMU();

  flatbuffers::FlatBufferBuilder* builder;
  uint8_t* msg_buf;
  unsigned int msg_size;
  USFS imu_sensor;
  double mag_declination = -14.42; // magnetic declination at MIT
};

AppIMU::AppIMU() {
}

AppIMU::~AppIMU() {
}

void AppIMU::CallBack(uint8_t* msg, size_t msg_size, std::string topic) {
  std::cout << "Callback received for topic \"" << topic << "\"" << std::endl;
  std::string msg_str(msg, msg+msg_size); // required for beaglebone
  if(topic == "nav.gps") {
    auto nav_gps_msg = topics::nav::Getgps(msg_str.data());
    auto time = nav_gps_msg->time();
    mag_declination = nav_gps_msg->mag_declination();
  } else {
    std::cout << "Unknown message \"" << topic << "\"" << std::endl;
  }
}

void AppIMU::SetMessageNavIMU() {
  builder->Clear();
  auto now     = std::chrono::system_clock::now();
  auto epoch   = now.time_since_epoch();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
  double utc = milliseconds.count()/1000.0;
  double val;

  // all string variables must be initialized separately
  std::string status;
  if(imu_sensor.algoStatus == 8) {
    status = "locked";
  } else if(imu_sensor.algoStatus == 0) {
    status = "converging";
  } else {
    status = "error";
  }
  auto str_status = builder->CreateString(status);

  topics::nav::imuBuilder imu_builder(*builder);
  imu_builder.add_time(utc);
  imu_builder.add_status(str_status);
  imu_builder.add_roll(imu_sensor.angle[0]);
  imu_builder.add_pitch(imu_sensor.angle[1]);
  imu_builder.add_yaw(imu_sensor.heading);
  imu_builder.add_quaternion_0(imu_sensor.qt[0]);
  imu_builder.add_quaternion_x(imu_sensor.qt[1]);
  imu_builder.add_quaternion_y(imu_sensor.qt[2]);
  imu_builder.add_quaternion_z(imu_sensor.qt[3]);
  imu_builder.add_accel_x(imu_sensor.accData[0]);
  imu_builder.add_accel_y(imu_sensor.accData[1]);
  imu_builder.add_accel_z(imu_sensor.accData[2]);
  imu_builder.add_gyro_x(imu_sensor.gyroData[0]);
  imu_builder.add_gyro_y(imu_sensor.gyroData[1]);
  imu_builder.add_gyro_z(imu_sensor.gyroData[2]);
  imu_builder.add_mag_x(imu_sensor.magData[0]);
  imu_builder.add_mag_y(imu_sensor.magData[1]);
  imu_builder.add_mag_z(imu_sensor.magData[2]);
  imu_builder.add_pressure(imu_sensor.pressure);
  imu_builder.add_temperature(imu_sensor.temperature);

  auto imu_msg = imu_builder.Finish();
  builder->Finish(imu_msg);
  msg_buf = builder->GetBufferPointer();
  msg_size = builder->GetSize();
}

void AppIMU::Init() {
  unsigned int tick = GetConfigParameter<unsigned int>("tick");
  SetHz(tick);

  imu_sensor.initSensors();

  builder = new flatbuffers::FlatBufferBuilder();

  // subscribe for nav.gps messages to get magnetic declination
  Subscribe("nav.gps", [this](uint8_t* msg, size_t msg_size, std::string topic){CallBack(msg, msg_size, topic);});
}

void AppIMU::Process() {
  imu_sensor.mag_declination = mag_declination;
  imu_sensor.FetchEventStatus();
  imu_sensor.FetchSentralData();
  SetMessageNavIMU();
  Publish("nav.imu", msg_buf, msg_size);
}

int main(int argc, char *argv[])
{
  if(argc < 2) {
    std::cout << "Expected a YAML configuration file... Exiting." << std::endl;
    exit(0);
  }
  std::string config_file = argv[1];
  AppIMU app_gps;
  app_gps.SetName("app_imu");
  app_gps.SetConfig(config_file);
  app_gps.Run();
}
