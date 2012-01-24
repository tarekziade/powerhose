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

        # poll
        self.poll = zmq.Poller()
        self.poll.register(self.main_controller, zmq.POLLIN)

    def stop(self):
        self.context.destroy(0)

    def num_workers(self):
        return int(self.call("NUMWORKERS"))

    def call(self, msg):
        self.main_controller.send(msg)

        res = self.poll.poll(self.timeout)
        if res == []:
            res = None
        else:
            res = self.main_controller.recv()

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
    try:
        print soaker.call(args.action.upper())
    except TimeoutError:
        print("The call timed out or the server is not online.")
    finally:
        soaker.stop()

    sys.exit()
