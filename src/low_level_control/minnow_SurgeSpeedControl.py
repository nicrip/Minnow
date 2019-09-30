#!/usr/bin/env python
import time
import math
import numpy as np

class speed_controller:

	# Speed to thrust map
	speed = [0, 0.4, 0.8, 1.2, 1.6, 2.0]
	thrust = [0, 10, 20, 30, 40, 50]
	speed_thrust_map = np.poly1d(np.polyfit(speed,thrust,2))

	def __init__(self, max_thrust = 100, min_thrust = -100):
		speed = [0, 0.4, 0.8, 1.2, 1.6, 2.0]
		thrust = [0, 10, 20, 30, 40, 50]
		self.max_thrust = max_thrust
		self.min_thrust = min_thrust
		self.speed_thrust_map = np.poly1d(np.polyfit(speed,thrust,2))

	def update(self,desired_speed):
		# Thrust output
		self.speed_thrust = self.speed_thrust_map(desired_speed)

		if self.speed_thrust > self.max_thrust:
			self.speed_thrust = self.max_thrust
		if self.speed_thrust < self.min_thrust:
			self.speed_thrust = self.min_thrust

		return(self.speed_thrust)

