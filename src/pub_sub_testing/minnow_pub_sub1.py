#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import zmq
import time
import signal
from threading import Thread

class Subscribe(Thread):
    def __init__(self, context, id, topic):
        super().__init__()
        self.context = context
        self.id = id
        self.topic = topic
        self.loop = False

    def run(self):
        print('starting subscriber {}, on topic {}'.format(self.id, self.topic))
        subscriber = self.context.socket(zmq.SUB)
        subscriber.connect("tcp://127.0.0.1:5555")
        subscriber.setsockopt_string(zmq.SUBSCRIBE, self.topic)
        poller = zmq.Poller()
        poller.register(subscriber, zmq.POLLIN)
        self.loop = True
        while self.loop:
            evts = poller.poll(1000)
            if evts:
                message = subscriber.recv()
                print('subscriber {}: {}'.format(self.id, message))

    def stop(self):
        self.loop = False

class Publish:
    def __init__(self):
        signal.signal(signal.SIGINT, self.exit_signal)
        self.zmq_context = zmq.Context()
        self.subscribers = []
        self.subscribe()
        self.run()

    def exit_signal(self, sig, frame):
        print('You pressed Ctrl+C!')
        for subscriber in self.subscribers:
            print('stopping subscriber {}'.format(subscriber.id))
            subscriber.stop()
        sys.exit(0)

    def subscribe(self):
        sub0 = Subscribe(self.zmq_context, 0, 'NAV')
        self.subscribers.append(sub0)
        sub0.start()

        sub1 = Subscribe(self.zmq_context, 1, 'PRESS')
        self.subscribers.append(sub1)
        sub1.start()

    def run(self):
        socket = self.zmq_context.socket(zmq.PUB)
        socket.connect("tcp://127.0.0.1:5556")

        while True:
            socket.send_string('POS' + time.strftime('%H:%M:%S'))

if __name__ == "__main__":
    pub = Publish()
