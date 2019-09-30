#!/usr/bin/env python
import time
import math
# import random
import numpy as np
# from scipy.optimize import least_squares
# import matplotlib.pyplot as plt
# import os, os.path
from minnow_HeadingControl import *  
from minnow_SurgeSpeedControl import *  

class control_system_manager:
	
	# This is for standalone troubleshooting of the python code ---------------------
	desired_speed = 1.6
	desired_heading = 60.0

	current_speed = 1.0
	current_heading = 30.0
	# -------------------------------------------------------------------------------

	speed_control_system = speed_controller()

	heading_control_system = heading_controller()
	heading_control_system.DesiredHeading(desired_heading)
	while True:
		# Run speed controller
		(speed_contrl_thrust)=speed_control_system.update(desired_speed)
		print("Speed Control Thrust Output: %f" % (speed_contrl_thrust))
		# Run heading controller
		heading_control_system.update(current_heading,speed_contrl_thrust)





