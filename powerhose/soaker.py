import time
import os
import binascii
import zmq
from  multiprocessing import Process
import threading
from collections import defaultdict
import sys
import argparse


class TimeoutError(Exception):
    pass


_DEFAULT = 'tcp://localhost:5555'


class Soaker(object):
    def __init__(self, host=_DEFAULT, timeout=5.):
        self.timeout = timeout
        self.host = host

        # Initialize a zeromq context
        self.context = zmq.Context()

        # set up the main controller
        self.main_controller = self.context.socket(zmq.REQ)
        self.main_controller.connect(self.host)

        # pinging the controller to check we're online
        res = self.call("PING")
        if res != "PONG":
            raise ValueError(res)

    def stop(self):
        self.main_controller.close()

    def num_workers(self):
        return int(self.call("NUMWORKERS"))

    def call(self, msg):
        self.main_controller.send(msg)
        start = time.time()
        res = None
        while time.time() - start < self.timeout:
            try:
                res = self.main_controller.recv(flags=zmq.NOBLOCK)
            except zmq.core.error.ZMQError:
                time.sleep(.05)
            else:
                break

        if res is None:
            raise TimeoutError()

        return res


def main(args=sys.argv):

    parser = argparse.ArgumentParser(description='Powerhose controller')

    parser.add_argument('--host', dest='host',
                        default='tcp://localhost:5555',
                        help='Server zmq channel')

    parser.add_argument('--timeout', dest='timeout', type=int,
                        default=5, help='Timeout')

    parser.add_argument('action', help='action to send')


    args = parser.parse_args()

    soaker = Soaker(args.host, args.timeout)
    print soaker.call(args.action.upper())
