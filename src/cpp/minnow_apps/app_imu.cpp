#include <iostream>
#include <sstream>
#include <math.h>
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
  std::vector<std::vector<double>> euler_offset;
  double psi, theta, phi, sy, roll, pitch, yaw;
  double R_00, R_01, R_02, R_10, R_11, R_12, R_20, R_21, R_22;
  double N_00, N_01, N_02, N_10, N_11, N_12, N_20, N_21, N_22;
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

  // apply euler offset to roll, pitch and yaw
  psi = (imu_sensor.angle[0])*M_PI/180.0;
  theta =(imu_sensor.angle[1])*M_PI/180.0;
  phi = (imu_sensor.heading)*M_PI/180.0;
  R_00 = cos(theta)*cos(phi);
  R_01 = sin(psi)*sin(theta)*cos(phi) - cos(psi)*sin(phi);
  R_02 = cos(psi)*sin(theta)*cos(phi) + sin(psi)*sin(phi);
  R_10 = cos(theta)*sin(phi);
  R_11 = sin(psi)*sin(theta)*sin(phi) + cos(psi)*cos(phi);
  R_12 = cos(psi)*sin(theta)*sin(phi) - sin(psi)*cos(phi);
  R_20 = -sin(theta);
  R_21 = sin(psi)*cos(theta);
  R_22 = cos(psi)*cos(theta);
  N_00 = euler_offset[0][0]*R_00 + euler_offset[1][0]*R_01 + euler_offset[2][0]*R_02;
  N_01 = euler_offset[0][1]*R_00 + euler_offset[1][1]*R_01 + euler_offset[2][1]*R_02;
  N_02 = euler_offset[0][2]*R_00 + euler_offset[1][2]*R_01 + euler_offset[2][2]*R_02;
  N_10 = euler_offset[0][0]*R_10 + euler_offset[1][0]*R_11 + euler_offset[2][0]*R_12;
  N_11 = euler_offset[0][1]*R_10 + euler_offset[1][1]*R_11 + euler_offset[2][1]*R_12;
  N_12 = euler_offset[0][2]*R_10 + euler_offset[1][2]*R_11 + euler_offset[2][2]*R_12;
  N_20 = euler_offset[0][0]*R_20 + euler_offset[1][0]*R_21 + euler_offset[2][0]*R_22;
  N_21 = euler_offset[0][1]*R_20 + euler_offset[1][1]*R_21 + euler_offset[2][1]*R_22;
  N_22 = euler_offset[2][2]*R_20 + euler_offset[1][2]*R_21 + euler_offset[2][2]*R_22;
  sy = sqrt(N_00*N_00 + N_10*N_10);
  if(sy >= 1e-6) {
    roll = (atan2(N_21, N_22))*180.0/M_PI;
    pitch = (atan2(-N_20, sy))*180.0/M_PI;
    yaw = (atan2(N_10, N_00))*180.0/M_PI;
  } else {
    roll = (atan2(-N_12, N_11))*180.0/M_PI;
    pitch = (atan2(-N_20, sy))*180.0/M_PI;
    yaw = 0.0;
  }
  if(yaw < 0.0) yaw += 360.0;

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

  // subscribe for nav.gps messages to get magnetic declination
  Subscribe("nav.gps", [this](uint8_t* msg, size_t msg_size, std::string topic){CallBack(msg, msg_size, topic);});

  euler_offset = GetConfigParameter<std::vector<std::vector<double>>>("euler_offset");

  imu_sensor.initSensors();

  builder = new flatbuffers::FlatBufferBuilder();
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
