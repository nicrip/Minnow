#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

import sys
import zmq
import time
import signal
import yaml
from threading import Thread, Event, Lock

class Subscriber(Thread):
    def __init__(self, app, address, topic, callback):
        super().__init__()
        self.app_ = app
        self.address_ = address
        self.topic_ = topic
        self.callback_ = callback
        self.mutex_ = Lock()
        self.new_msg_ = False
        self.msg_ = None
        self.zmq_context_ = app.zmq_context_
        self.receive_active_ = Event()

    def run(self):
        print("[T] Starting subscriber thread for topic: {}".format(self.topic_))
        subscriber = self.zmq_context_.socket(zmq.SUB)
        subscriber.connect(self.address_)
        subscriber.setsockopt_string(zmq.SUBSCRIBE, self.topic_)
        poller = zmq.Poller()
        poller.register(subscriber, zmq.POLLIN)
        while not self.receive_active_.is_set():
            evts = poller.poll(1000)
            if evts:
                message = subscriber.recv()

                topic_msg = message.split(b'_',1)
                topic = topic_msg[0].decode('UTF-8')
                if topic == self.topic_:
                    self.mutex_.acquire()
                    self.new_msg_ = True
                    self.msg_ = message
                    self.mutex_.release()
                else:
                    subscribed = self.app_.check_subscription_to_topic(topic)
                    if not subscribed:
                        self.app_.subscribe(topic, self.callback_)

        print("[T] Stopping subscriber thread for topic: {}".format(self.topic_))

class App:
    def __init__(self):
        self.zmq_context_ = zmq.Context()
        self.zmq_socket_ = self.zmq_context_.socket(zmq.PUB)
        self.mutex_ = Lock()
        self.subscriptions_ = []
        self.subscription_topics_ = []
        self.sleep_us_ = 10000  # 100Hz default tick

        self.name_ = None
        self.config_ = None
        self.protocol_ = None
        self.host_ = None
        self.port_ = None
        self.subscriber_address_ = None
        self.loop_start_ = None
        self.loop_end_ = None
        self.loop_us_ = None

        signal.signal(signal.SIGINT, self.exit_signal)

    def exit_signal(self, sig, frame):
        for subscriber in self.subscriptions_:
            subscriber.receive_active_.set()
        time.sleep(1)
        for subscriber in self.subscriptions_:
            print("[M] Subscription for {} killed.".format(subscriber.topic_))
            subscriber.join()
        time.sleep(1.5)
        print("Exiting Minnow App...")
        sys.exit(0)

    def subscribe(self, topic, callback):
        print("[M] Subscription for {} created.".format(topic))
        subscriber = Subscriber(self, self.subscriber_address_, topic, callback)
        self.mutex_.acquire()
        self.subscriptions_.append(subscriber)
        self.subscription_topics_.append(topic)
        self.mutex_.release()
        subscriber.start()

    def check_subscription_to_topic(self, topic):
        subscribed = False
        for subscribed_topic in self.subscription_topics_:
            if subscribed_topic == topic:
                subscribed = True
                break
        return subscribed

    def check_subscriptions(self):
        self.mutex_.acquire()
        for subscriber in self.subscriptions_:
            if subscriber.new_msg_:
                print("[M] New message from {}.".format(subscriber.topic_))
                subscriber.mutex_.acquire()
                subscriber.new_msg_ = False
                topic_msg = subscriber.msg_.split(b'_',1)
                subscriber.mutex_.release()
                topic = topic_msg[0].decode('UTF-8')
                msg = topic_msg[1]
                subscriber.callback_(msg, topic)
        self.mutex_.release()

    def publish_string(self, topic, msg):
        self.zmq_socket_.send_string(topic + '_' + msg)

    def publish(self, topic, msg):
        self.zmq_socket_.send(topic.encode('UTF-8') + b'_' + msg)

    def set_name(self, name):
        self.name_ = name

    def set_config(self, config_file):
        try:
            with open(config_file, 'r') as file:
                self.config_ = yaml.load(file)
        except:
            print("[C] Configuration file \"{}\" not found... Exiting.".format(config_file))
            self.exit_signal(None, None)

        try:
            self.protocol_ = self.config_["protocol"]
            self.host_ = self.config_["host"]
            if self.protocol_ == "ipc":
                print("[C] Using inter-process communications (ipc).")
            elif self.protocol_ == "tcp":
                print("[C] Using transmission control protocol (tcp).")
                self.port_ = int(self.config_["port"])
            else:
                print("[C] Unknown communications protocol... Exiting.")
                self.exit_signal(None, None)
        except:
            print("[C] Configuration file must define \"protocol\", \"host\" and, if required, \"port\"... Exiting.")
            self.exit_signal(None, None)

    def get_config_parameter(self, type, config_param):
        try:
            val = self.config_[self.name_][config_param]
        except:
            print("[C] Configuration parameter \"{}\" for Minnow App \"{}\" not found... Exiting.".format(config_param, self.name_))
            self.exit_signal(None, None)
        try:
            val = type(val)
        except:
            print("[C] Configuration parameter \"{}\" for Minnow App \"{}\" could not be cast into type \"{}\"... Exiting.".format(config_param, self.name_, type))
            self.exit_signal(None, None)
        return val

    def set_hz(self, tick_hz):
        if tick_hz < 0:
            print("[C] Configuration parameter \"tick\" for Minnow App \"{}\" must be positive or zero... Exiting.".format(self.name_))
        elif tick_hz == 0:
            self.sleep_us_ = 0
        else:
            self.sleep_us_ = int(1e6/tick_hz)

    def init(self): # must be implemented by subclasses
        for subscriber in self.subscriptions_:
            subscriber.receive_active_.set()
        time.sleep(1)
        for subscriber in self.subscriptions_:
            print("[M] Subscription for {} killed.".format(subscriber.topic_))
            subscriber.join()
        time.sleep(1.5)
        print("Exiting Minnow App...")
        raise NotImplementedError('Inheriting apps must implement init() method!')

    def process(self): # must be implemented by subclasses
        for subscriber in self.subscriptions_:
            subscriber.receive_active_.set()
        time.sleep(1)
        for subscriber in self.subscriptions_:
            print("[M] Subscription for {} killed.".format(subscriber.topic_))
            subscriber.join()
        time.sleep(1.5)
        print("Exiting Minnow App...")
        raise NotImplementedError('Inheriting apps must implement process() method!')

    def run(self):
        print("[M] Starting Minnow App \"{}\"".format(self.name_))

        if self.protocol_ == "ipc":
            socket = "ipc://" + self.host_ + ".sub"
        elif self.protocol_ == "tcp":
            socket = "tcp://" + self.host_ + ":" + self.port_+1
        print("[C] Publishing address: {}".format(socket))
        self.zmq_socket_.connect(socket)
        if self.protocol_ == "ipc":
            self.subscriber_address_ = "ipc://" + self.host_ + ".pub"
        elif self.protocol_ == "tcp":
            self.subscriber_address_ = "tcp://" + self.host_ + ":" + self.port_
        print("[C] Subscribing address: {}".format(self.subscriber_address_))

        self.init()

        while True:
            if self.sleep_us_ != 0:
                self.loop_start_ = time.time()

            self.check_subscriptions()
            self.process()

            if self.sleep_us_ != 0:
                self.loop_end_ = time.time()
                self.loop_us_ = 1e6*(self.loop_end_ - self.loop_start_)
                rem_usecs = self.sleep_us_ - self.loop_us_
                if rem_usecs > 0:
                    time.sleep(rem_usecs/1e6)
