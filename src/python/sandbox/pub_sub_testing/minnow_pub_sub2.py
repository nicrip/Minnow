#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import zmq
import time
import signal
from collections import deque
from threading import Thread

class Subscriber(Thread):
    def __init__(self, context, id, topics_callbacks):
        super().__init__()
        self.context = context
        self.topics_callbacks = topics_callbacks
        self.loop = False

    def run(self):
        print('starting subscriber, on topic(s) {}'.format(self.topics_callbacks.keys()))
        subscriber = self.context.socket(zmq.SUB)
        subscriber.connect("tcp://127.0.0.1:5555")
        for topic in self.topics_callbacks.keys():
            subscriber.setsockopt_string(zmq.SUBSCRIBE, topic)
        poller = zmq.Poller()
        poller.register(subscriber, zmq.POLLIN)
        self.loop = True
        while self.loop:
            evts = poller.poll(1000)
            if evts:
                message = subscriber.recv()
                for topic in self.topics_callbacks.keys():
                    if topic in str(message):
                        self.topics_callbacks[topic](message)

    def stop(self):
        self.loop = False

class Publisher:
    def __init__(self):
        signal.signal(signal.SIGINT, self.exit_signal)
        self.zmq_context = zmq.Context()
        self.setup_subscriber()
        self.pos_msg = None

    def exit_signal(self, sig, frame):
        print('You pressed Ctrl+C!')
        self.subscriber.stop()
        time.sleep(1)
        self.subscriber.join()
        sys.exit(0)

    def setup_subscriber(self):
        self.subscriber = Subscriber(self.zmq_context, 0, {'POS':self.pos_callback})
        self.subscriber.start()

    def pos_callback(self, msg):
        self.pos_msg = msg

    def run(self):
        socket = self.zmq_context.socket(zmq.PUB)
        socket.connect("tcp://127.0.0.1:5556")

        count = 0
        while True:
            count += 1
            socket.send_string('NAV_' + str(count))
            socket.send_string('PRESS_' + str(count))
            if count%100 == 0:
                print(self.pos_msg)
            time.sleep(0.0001)

if __name__ == "__main__":
    pub = Publisher()
    pub.run()
