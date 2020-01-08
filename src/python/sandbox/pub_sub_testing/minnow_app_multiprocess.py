#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import zmq
import time
import signal
from collections import deque
from multiprocessing import Process, Queue, Event

class Subscriber(Process):
    def __init__(self, topic, callback):
        super().__init__()
        self.queue = Queue(maxsize=1)
        self.context = zmq.Context()
        self.topic = topic
        self.callback = callback
        self.shutdown_flag = False
        self.start()

    def run(self):
        print('starting subscriber, on topic {}'.format(self.topic))
        subscriber = self.context.socket(zmq.SUB)
        subscriber.connect("tcp://127.0.0.1:5555")
        subscriber.setsockopt_string(zmq.SUBSCRIBE, self.topic)
        poller = zmq.Poller()
        poller.register(subscriber, zmq.POLLIN)
        while not self.shutdown_flag:
            evts = poller.poll(1000)
            if evts and not self.shutdown_flag:
                message = subscriber.recv()
                self.queue.put(message)

    def stop(self):
        self.shutdown_flag = True
        print('subscriber stopped, on topic {}'.format(self.topic))
        exit()

class App:
    def __init__(self):
        signal.signal(signal.SIGINT, self.exit_signal)
        self.zmq_context = zmq.Context()
        self.socket = self.zmq_context.socket(zmq.PUB)
        self.subscriptions = []

    def exit_signal(self, sig, frame):
        print('You pressed Ctrl+C!')
        for subscriber in self.subscriptions:
            subscriber.stop()
        time.sleep(1)
        for subscriber in self.subscriptions:
            subscriber.join()
        sys.exit(0)

    def subscribe(self, topic, callback):
        subscriber = Subscriber(topic, callback)
        self.subscriptions.append(subscriber)

    def check_subscriptions(self):
        for subscriber in self.subscriptions:
            if not subscriber.queue.empty():
                msg = subscriber.queue.get()
                subscriber.callback(msg)

    def publish(self, msg):
        self.socket.send_string(msg)

    def process(self):
        for subscriber in self.subscriptions:
            subscriber.shutdown_flag.set()
        time.sleep(1)
        for subscriber in self.subscriptions:
            subscriber.join()
        raise NotImplementedError('Inheriting apps must implement process() method!')

    def run(self):
        self.socket.connect("tcp://127.0.0.1:5556")
        while True:
            self.check_subscriptions()
            self.process()
