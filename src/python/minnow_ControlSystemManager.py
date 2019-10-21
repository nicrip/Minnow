#!/usr/bin/env python
import time
import math
import numpy as np
from minnow_low_level_control.HeadingControl import *   
from minnow_low_level_control.SurgeSpeedControl import *   

class control_system_manager:
	
	# This is for standalone troubleshooting of the python code ---------------------
	desired_speed = 1.0
	desired_heading = -160.0	# between -180 and 180 (i.e. 181 = -179)
	desired_pitch = 0.0

	current_speed = 1.0
	current_heading = 170
	current_pitch = -2.0
	# -------------------------------------------------------------------------------

	if (desired_heading >= 0) and (desired_heading <= 180):
		contrl_desired_heading = desired_heading
	else:
		contrl_desired_heading = desired_heading - 360

	if (current_heading >= 0) and (current_heading <= 180):
		contrl_current_heading = current_heading
	else:
		contrl_current_heading = current_heading - 360


	speed_control_system = speed_controller()
	heading_control_system = heading_controller()
	heading_control_system.DesiredHeading(contrl_desired_heading)
	while True:
		# Run speed controller
		(speed_contrl_thrust)=speed_control_system.update(desired_speed)
		print("Speed Control Thrust Output: %f" % (speed_contrl_thrust))
		# Run heading controller
		(hdg_differential_thrust,hdg_port_thrust,hdg_stbd_thrust)=heading_control_system.update(contrl_current_heading,speed_contrl_thrust)
		print("Heading control port thrust: %f" % hdg_port_thrust)
		print("Heading control stbd thrust: %f" % hdg_stbd_thrust)


		# Final motor thrust computation

		time.sleep(.01)





