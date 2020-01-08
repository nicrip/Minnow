#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

import sys
import zmq
import time
import signal
from collections import deque
from threading import Thread, Event
from queue import Queue

class Subscriber(Thread):
    def __init__(self, context, topic, callback):
        super().__init__()
        self.queue = Queue(maxsize=1)
        self.context = context
        self.topic = topic
        self.callback = callback
        self.shutdown_flag = Event()
        self.start()

    def run(self):
        print('starting subscriber, on topic {}'.format(self.topic))
        subscriber = self.context.socket(zmq.SUB)
        subscriber.connect("tcp://127.0.0.1:5555")
        subscriber.setsockopt_string(zmq.SUBSCRIBE, self.topic)
        poller = zmq.Poller()
        poller.register(subscriber, zmq.POLLIN)
        while not self.shutdown_flag.is_set():
            evts = poller.poll(1000)
            if evts and not self.shutdown_flag.is_set():
                message = subscriber.recv()
                topic_name_msg = message.split(None,1)
                topic_name = topic_name_msg[0]
                topic_msg = topic_name_msg[1]
                self.queue.put(topic_msg)
        print('subscriber stopped, on topic {}'.format(self.topic))

class App:
    def __init__(self):
        signal.signal(signal.SIGINT, self.exit_signal)
        self.zmq_context = zmq.Context()
        self.socket = self.zmq_context.socket(zmq.PUB)
        self.subscriptions = []

    def exit_signal(self, sig, frame):
        print('You pressed Ctrl+C!')
        for subscriber in self.subscriptions:
            subscriber.shutdown_flag.set()
        time.sleep(1)
        for subscriber in self.subscriptions:
            subscriber.join()
        sys.exit(0)

    def subscribe(self, topic, callback):
        subscriber = Subscriber(self.zmq_context, topic, callback)
        self.subscriptions.append(subscriber)

    def check_subscriptions(self):
        for subscriber in self.subscriptions:
            if not subscriber.queue.empty():
                msg = subscriber.queue.get()
                subscriber.callback(msg)

    def publish(self, msg):
        self.socket.send(msg)

    def publish_string(self, msg):
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
