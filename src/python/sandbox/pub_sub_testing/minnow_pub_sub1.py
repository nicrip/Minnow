#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import zmq
import time
# from minnow_app_threaded import App
from minnow_app_multiprocess import App

class Publisher(App):
    def __init__(self):
        super().__init__()
        self.setup_subscribers()
        self.nav_msg = None
        self.press_msg = None
        self.count = 0

    def setup_subscribers(self):
        self.subscribe('NAV', self.nav_callback)
        self.subscribe('PRESS', self.press_callback)

    def nav_callback(self, msg):
        self.nav_msg = msg

    def press_callback(self, msg):
        self.press_msg = msg

    def process(self):
        self.count += 1
        self.publish('POS_' + str(self.count))
        if self.count%100 == 0:
            print(self.nav_msg, self.press_msg)
        time.sleep(0.001)

if __name__ == "__main__":
    pub = Publisher()
    pub.run()
