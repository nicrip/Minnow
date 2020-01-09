#include <iostream>
#include <zmq.hpp>

#ifndef MINNOW_APP_THREADED_HDR
#define MINNOW_APP_THREADED_HDR

class App
{
 public:
   App();
   ~App();

 protected:
   void ExitSignal(int s);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();

 private: // Configuration variables
   double       m_valid_std_azimuth;
   double       m_valid_std_range;
   double       m_loiter_x;
   double       m_loiter_y;
   // Beacon location estimate published by pLamssMissionManager
   double       m_aco_beacon_x;
   double       m_aco_beacon_y;
   std::string  m_return_update_var;
   std::string  m_abort_update_var;

 private: // State variables
   double       m_target_x;
   double       m_target_y;
   double       m_std_azimuth;
   double       m_std_range;
   std::string  m_deploy_mission;
   std::string  m_mission_state;
   double       m_nav_x;
   double       m_nav_y;
   double       m_nav_speed;
   unsigned int m_iterations;
   double       m_timewarp;
};

#endif
