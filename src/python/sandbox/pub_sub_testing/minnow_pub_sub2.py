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
        self.pos_msg = None
        self.count = 0

    def setup_subscribers(self):
        self.subscribe('POS', self.pos_callback)

    def pos_callback(self, msg):
        self.pos_msg = msg

    def process(self):
        self.count += 1
        self.publish('NAV_' + str(self.count))
        self.publish('PRESS_' + str(self.count))
        if self.count%100 == 0:
            print(self.pos_msg)
        time.sleep(0.001)

if __name__ == "__main__":
    pub = Publisher()
    pub.run()
