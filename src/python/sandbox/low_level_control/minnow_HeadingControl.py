#!/usr/bin/env python
import time
import math
# import random
import numpy as np
# from scipy.optimize import least_squares
# import matplotlib.pyplot as plt
# import os, os.path 

class heading_controller:
	
	def __init__(self, P=10.0, I=0.0, D=1.0, prev_hdg_error=0, hdg_error_integral=0, max_hdg_error_integral=50):

		self.hdg_kp=P
		self.hdg_ki=I
		self.hdg_kd=D
		self.prev_hdg_error=prev_hdg_error
		self.hdg_error_integral=hdg_error_integral
		self.max_hdg_error_integral=max_hdg_error_integral
		self.desired_heading=0.0
		self.error=0.0

	def update(self,current_heading,speed_thrust):
		self.hdg_error = self.desired_heading - current_heading

		# Setting the integral of the error
		self.hdg_error_integral = self.hdg_error_integral + self.hdg_error
		if self.hdg_error_integral > self.max_hdg_error_integral:
			self.hdg_error_integral = self.max_hdg_error_integral
		if self.hdg_error_integral < -self.max_hdg_error_integral:
			self.hdg_error_integral = -self.max_hdg_error_integral

		hdg_p_value = self.hdg_kp * self.hdg_error
		hdg_i_value = self.hdg_ki * (self.hdg_error_integral)
		hdg_d_value = self.hdg_kd * (self.hdg_error - self.prev_hdg_error)

		differential_thrust = hdg_p_value + hdg_i_value + hdg_d_value


		print(differential_thrust)


		# For error deravative 
		self.prev_hdg_error = self.hdg_error

	def DesiredHeading(self,desired_heading):
		self.desired_heading = desired_heading
		self.hdg_error_integral=0
		self.prev_hdg_error=0








	# print(speed_controller.speed_contrl_port_thrust)






hdg_contrl_port_thrust = 50
hdg_contrl_port_thrust = 0