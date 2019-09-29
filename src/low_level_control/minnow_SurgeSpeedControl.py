#!/usr/bin/env python
import time
import math
import random
import numpy as np
from scipy.optimize import least_squares
import matplotlib.pyplot as plt
import os, os.path

# This is for standalone troubleshooting of the python code ---------------------
desired_speed = 1.5 # m/s
current_speed = 0.0	# m/s - this will not be used for now as currently speed control is open loop
# -------------------------------------------------------------------------------

# Speed to thrust map