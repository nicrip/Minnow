#!/usr/bin/env python

# Overall control config
config_max_motor_thrust = 80
config_min_motor_thrust = -80

# Heading control configs
config_hdg_kp = 1.2
config_hdg_ki = 0.0
config_hdg_kd = 0.0
config_max_hdg_error_integral = 50

# Speed control config
config_map_speed = [0, 0.4, 0.8, 1.2, 1.6, 2.0]		# thrust to speed map
config_map_thrust = [0, 10, 20, 30, 40, 50]			# thrust to speed map

