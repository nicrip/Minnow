#!/usr/bin/env python
import time
import math
import random
import numpy as np
from scipy.optimize import least_squares
import matplotlib.pyplot as plt
import os, os.path
from minnow_SurgeSpeedControl import *  

# This is for standalone troubleshooting of the python code ---------------------
desired_heading = 60.0
current_heading = 30.0
# -------------------------------------------------------------------------------

# speed control - speed map







# Output values
thrust_port = desired_speed * 10
thrust_stbd = 50
thrust_vert = 0