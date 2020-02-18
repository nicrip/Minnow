#!/usr/bin/env python
import time
import math
from .controlconfig import * 

class heading_controller:
    def __init__(self, prev_hdg_error=0, hdg_error_integral=0):
        self.hdg_kp = config_hdg_kp
        self.hdg_ki = config_hdg_ki
        self.hdg_kd = config_hdg_kd
        self.max_hdg_error_integral=config_max_hdg_error_integral

        self.prev_hdg_error=prev_hdg_error
        self.hdg_error_integral=hdg_error_integral
        self.desired_heading=0.0
        self.hdg_error=0.0
        self.prev_hdg_stbd_thrust = 0.0
        self.prev_hdg_port_thrust = 0.0

    def update(self,current_heading,speed_thrust):
        # Calculating heading error
        # +ve errors turn towards stbd and -ve towards port
        self.hdg_error = self.desired_heading - current_heading
        if (self.hdg_error>=180):
            self.hdg_error=(self.hdg_error-360)
        elif (self.hdg_error<=-180):
            self.hdg_error=(self.hdg_error+360)
        print('Current heading',current_heading)
        print('Desired heading',self.desired_heading)
        print('Heading error',self.hdg_error)

        # Setting the integral of the error
        self.hdg_error_integral = self.hdg_error_integral + self.hdg_error
        if self.hdg_error_integral > self.max_hdg_error_integral:
            self.hdg_error_integral = self.max_hdg_error_integral
        if self.hdg_error_integral < -self.max_hdg_error_integral:
            self.hdg_error_integral = -self.max_hdg_error_integral

        hdg_p_value = self.hdg_kp * self.hdg_error
        hdg_i_value = self.hdg_ki * (self.hdg_error_integral)
        hdg_d_value = self.hdg_kd * (self.hdg_error - self.prev_hdg_error)
        hdg_differential_thrust = hdg_p_value + hdg_i_value + hdg_d_value
        print('Heading differential thrust ',hdg_differential_thrust)

        # mixing speed thrust and diff thrust
        hdg_port_thrust = speed_thrust + 0.5 *hdg_differential_thrust
        hdg_stbd_thrust = speed_thrust - 0.5 *hdg_differential_thrust
        if (speed_thrust + 0.5 *abs(hdg_differential_thrust)) > config_max_motor_thrust:
            hdg_differential_thrust_correction = (speed_thrust + 0.5 *abs(hdg_differential_thrust)) - config_max_motor_thrust
            hdg_port_thrust = hdg_port_thrust - hdg_differential_thrust_correction
            hdg_stbd_thrust = hdg_stbd_thrust - hdg_differential_thrust_correction

        # elif (speed_thrust + 0.5 *hdg_differential_thrust) < config_min_motor_thrust:
        #     hdg_differential_thrust_correction = (speed_thrust + 0.5 *hdg_differential_thrust) - config_min_motor_thrust
        #     print(hdg_differential_thrust_correction)
        #     hdg_port_thrust = hdg_port_thrust - hdg_differential_thrust_correction
        #     hdg_stbd_thrust = hdg_stbd_thrust - hdg_differential_thrust_correction

        # motor safelty limits
        if hdg_port_thrust > config_max_motor_thrust:
            hdg_port_thrust = config_max_motor_thrust
        if hdg_stbd_thrust > config_max_motor_thrust:
            hdg_stbd_thrust = config_max_motor_thrust
        if hdg_port_thrust < config_min_motor_thrust:
            hdg_port_thrust = config_min_motor_thrust
        if hdg_stbd_thrust < config_min_motor_thrust:
            hdg_stbd_thrust = config_min_motor_thrust

        if (abs(hdg_stbd_thrust-self.prev_hdg_stbd_thrust)>80):
            hdg_stbd_thrust = 0.25*hdg_stbd_thrust
        if (abs(hdg_port_thrust-self.prev_hdg_port_thrust)>80):
            hdg_port_thrust = 0.25*hdg_port_thrust

        # For error deravative
        self.prev_hdg_error = self.hdg_error
        self.prev_hdg_stbd_thrust = hdg_stbd_thrust
        self.prev_hdg_port_thrust = hdg_port_thrust

        return(hdg_port_thrust,hdg_stbd_thrust)

    def DesiredHeading(self,desired_heading):
        self.desired_heading = desired_heading
        self.hdg_error_integral=0
        self.prev_hdg_error=0
