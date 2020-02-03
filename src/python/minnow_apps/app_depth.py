#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

import sys
import zmq
import time
# minnow comms
from minnow_comms.minnow_app_threaded import App
# flatbuffer serialization
import flatbuffers
# generated by flatc
import topics.nav.depth
# depth driver MS5837
from minnow_drivers.pyMS5837 import pyMS5837

class AppDepth(App):
    def __init__(self):
        super().__init__()
        self.builder = None
        self.msg = None
        self.depth_sensor = None

    def set_message_nav_depth(self):
        topics.nav.depth.depthStart(self.builder)
        topics.nav.depth.depthAddTime(self.builder, time.time())
        topics.nav.depth.depthAddTemperature(self.builder, self.depth_sensor.get_temperature(pyMS5837.UNITS_Celsius))
        topics.nav.depth.depthAddPressure(self.builder, self.depth_sensor.get_pressure(pyMS5837.UNITS_mbar))
        topics.nav.depth.depthAddDepthFluid(self.builder, self.depth_sensor.get_depth())

        print('Temperature: {:6.2f} C'.format(self.depth_sensor.get_temperature(pyMS5837.UNITS_Celsius)))
        print('Pressure: {:6.2f} mbar'.format(self.depth_sensor.get_pressure(pyMS5837.UNITS_mbar)))
        print('Depth: {:6.2f} m'.format(self.depth_sensor.get_depth()))
        print('')

        depth_msg = topics.nav.depth.depthEnd(self.builder)
        self.builder.Finish(depth_msg)
        self.msg = self.builder.Output()

    def init(self):
        tick = self.get_config_parameter(int, "tick")
        self.set_hz(tick)

        fluid_density = self.get_config_parameter(float, "fluid_density")
        oversampling = self.get_config_parameter(int, "oversampling")
        self.depth_sensor = pyMS5837.MS5837(2, model=pyMS5837.MODEL_30BA, fluid_density=fluid_density, oversampling=oversampling)

        self.builder = flatbuffers.Builder(1024)

    def process(self):
        self.depth_sensor.read()
        self.set_message_nav_depth()
        self.publish('nav.depth', self.msg)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Expected a YAML configuration file... Exiting.")
        sys.exit()
    else:
        config_file = sys.argv[1]
    app_depth = AppDepth()
    app_depth.set_name("app_depth")
    app_depth.set_config(config_file)
    app_depth.run()
