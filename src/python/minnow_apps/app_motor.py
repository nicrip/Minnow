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
import topics.motor.command
import topics.motor.value
# PWM library
from Adafruit_BBIO import PWM

class AppMotor(App):
    def __init__(self):
        super().__init__()
        self.builder = None
        self.msg = None
        self.motor_command_msg = None
        self.motor_port_command = None
        self.motor_starboard_command = None
        self.motor_upper_command = None
        self.pwm_freq = None                                                    #8000 Hz max for BlueRobotics ESC
        self.pwm_stop = None                                                    #motor esc stops at 1500 microseconds
        self.pwm_min = None                                                     #max reverse microseconds pwm value
        self.pwm_max = None                                                     #max forward microseconds pwm value
        self.pwm_deadband = None                                                #deadband around 1500 microseconds of 40 microseconds
        self.command_min = None
        self.command_max = None
        self.command_deadband = None

    def motor_command_callback(self, msg, topic):
        print("Callback received for topic \"{}\"".format(topic))
        if topic == "motor.command":
            self.motor_command_msg = topics.nav.gps.gps.GetRootAsgps(msg, 0)
            self.motor_port_command = self.motor_command_msg.MotorPortCommand()
            self.motor_starboard_command = self.motor_command_msg.MotorStarboardCommand()
            self.motor_upper_command = self.motor_command_msg.MotorUpperCommand()
        else:
            print("Unknown message \"{}\"".format(topic))

    def set_message_motor_value(self):
        # limit motor commands and map to pwm values
        if self.motor_port_command < self.command_min:
            self.motor_port_command = self.command_min
        if self.motor_port_command > self.command_max:
            self.motor_port_command = self.command_max
        motor_port_pwm = self.map_command_to_pwm(self.motor_port_command)
        if self.motor_starboard_command < self.command_min:
            self.motor_starboard_command = self.command_min
        if self.motor_starboard_command > self.command_max:
            self.motor_starboard_command = self.command_max
        motor_starboard_pwm = self.map_command_to_pwm(self.motor_starboard_command)
        if self.motor_upper_command < self.command_min:
            self.motor_upper_command = self.command_min
        if self.motor_upper_command > self.command_max:
            self.motor_upper_command = self.command_max
        motor_upper_pwm = self.map_command_to_pwm(self.motor_upper_command)
        #set duty cycles
        PWM.set_duty_cycle(self.motor_port_pin, motor_port_pwm)
        PWM.set_duty_cycle(self.motor_starboard_pin, motor_starboard_pin)
        PWM.set_duty_cycle(self.motor_upper_pin, motor_upper_pwm)

        topics.motor.value.valueStart(self.builder)
        topics.motor.value.valueAddTime(self.builder, time.time())
        topics.motor.value.valueAddMotorPortValue(self.builder, motor_port_pwm)
        topics.motor.value.valueAddMotorStarboardValue(self.builder, motor_starboard_pin)
        topics.motor.value.valueAddMotorUpperValue(self.builder, motor_upper_pwm)

        print('Motor port value: {:6.3f}'.format(motor_port_pwm))
        print('Motor starboard value: {:6.3f}'.format(motor_starboard_pin))
        print('Motor upper value: {:6.3f}'.format(motor_upper_pwm))
        print('')

        value_msg = topics.motor.value.valueEnd(self.builder)
        self.builder.Finish(value_msg)
        self.msg = self.builder.Output()

    def init(self):
        tick = self.get_config_parameter(int, "tick")
        self.set_hz(tick)

        self.subscribe('motor.command', self.motor_command_callback)            # subscribe to motor command messages

        self.motor_port_pin = self.get_config_parameter(str, "motor_port_pin")
        self.motor_starboard_pin = self.get_config_parameter(str, "motor_starboard_pin")
        self.motor_upper_pin = self.get_config_parameter(str, "motor_upper_pin")
        self.pwm_freq = self.get_config_parameter(float, "pwm_freq")
        self.pwm_stop = self.get_config_parameter(float, "pwm_stop")
        self.pwm_min = self.get_config_parameter(float, "pwm_min")
        self.pwm_max = self.get_config_parameter(float, "pwm_max")
        self.pwm_deadband = self.get_config_parameter(float, "pwm_deadband")
        self.command_min = self.get_config_parameter(float, "command_min")
        self.command_max = self.get_config_parameter(float, "command_max")
        self.command_deadband = self.get_config_parameter(float, "command_deadband")

        PWM.start(self.motor_port_pin, self.pwm_stop/(1e6/self.pwm_freq), self.pwm_freq)
        PWM.start(self.motor_starboard_pin, self.pwm_stop/(1e6/self.pwm_freq), self.pwm_freq)
        PWM.start(self.motor_upper_pin, self.pwm_stop/(1e6/self.pwm_freq), self.pwm_freq)
        time.sleep(10.0)

        self.builder = flatbuffers.Builder(1024)

    def exit_signal(self, sig, frame):
        PWM.stop(self.motor_port_pin)
        PWM.stop(self.motor_starboard_pin)
        PWM.stop(self.motor_upper_pin)
        PWM.cleanup()
        super().exit_signal()

    def map_command_to_pwm(self, command):
        if abs(command) <= self.command_deadband:                               #stop vehicle if command is within command deadband
            return self.pwm_stop/(1e6/self.pwm_freq)
        else:
            command_scale = (command - self.command_min)/(self.command_max - self.command_min)
            min_val = self.pwm_min/(1e6/self.pwm_freq)
            max_val = self.pwm_max/(1e6/self.pwm_freq)
            pwm_val = command_scale*(max_val - min_val) + min_val
            pwm_val_us = pwm_val*(1e6/self.pwm_freq)
            if abs(pwm_val_us - self.pwm_stop) <= self.pwm_deadband:            #if pwm mapping is within pwm deadband, push outside pwm deadband
                if abs(pwm_val_us - (self.pwm_stop - self.pwm_deadband)) < abs(pwm_val_us - (self.pwm_stop + self.pwm_deadband)):
                    pwm_val = (self.pwm_stop-self.pwm_deadband)/(1e6/self.pwm_freq)
                else:
                    pwm_val = (self.pwm_stop+self.pwm_deadband)/(1e6/self.pwm_freq)
            return pwm_val

    def process(self):
        self.set_message_motor_value()
        self.publish('motor.value', self.msg)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Expected a YAML configuration file... Exiting.")
        sys.exit()
    else:
        config_file = sys.argv[1]
    app_motor = AppMotor()
    app_motor.set_name("app_motor")
    app_motor.set_config(config_file)
    app_motor.run()
