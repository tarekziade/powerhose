import time
import os
import binascii
import zmq
from  multiprocessing import Process
import threading
from collections import defaultdict
import sys


class TimeoutError(Exception):
    pass


class Receiver(threading.Thread):
    """Receive results asynchronously
    """
    def __init__(self, context, timeout):
        threading.Thread.__init__(self)
        self.timeout = timeout
        self.receiver = context.socket(zmq.PULL)
        self.receiver.bind(RES)
        self.running = False
        self._callbacks = defaultdict(list)

    def stop(self):
        self.running = False
        time.sleep(0.1)
        self.receiver.close()
        #self.join(timeout=1.)

    def register(self, callback, job_id):
        self._callbacks[job_id].append(callback)

    def run(self):
        self.running = True

        while self.running:
            try:
                res = self.receiver.recv(flags=zmq.NOBLOCK)
            except zmq.core.error.ZMQError:
                time.sleep(.05)
                continue

            # the data we get back is composed of 3 fields
            # - a job id
            # - a status (OK or KO)
            # - extra data that can be the result or a traceback
            job_id, data = res.split(':', 1)
            status, data = data.split(':', 1)

            # the callbacks are called with another thread.
            for callback in self._callbacks[job_id]:
                try:
                    callback(job_id, status, data)
                except Exception, e:
                    # log this
                    pass


WORK = 'ipc:///tmp/sender'
RES = 'ipc:///tmp/receiver'
CTR = 'ipc:///tmp/controller'
MAIN = 'ipc:///tmp/main'


class Sender(object):
    """ Use this class to send jobs.
    """
    def __init__(self, timeout=5.):
        self.timeout = timeout

        # Initialize a zeromq context
        self.context = zmq.Context()

        # set up the main controller
        self.main_controller = self.context.socket(zmq.REQ)
        self.main_controller.connect(MAIN)

        # pinging the controller to check we're online
        res = self.main_control("PING")
        if res != "PONG":
            raise ValueError(res)

        # Set up a channel to receive results
        self.receiver = Receiver(self.context, self.timeout)
        self.receiver.start()

        # Set up a channel to send work
        self.sender = self.context.socket(zmq.PUSH)
        self.sender.bind(WORK)

    def stop(self):
        self.main_controller.close()
        self.sender.close()
        self.receiver.stop()

    def num_workers(self):
        return int(self.main_control("NUMWORKERS"))

    def main_control(self, msg):
        self.main_controller.send(msg)
        start = time.time()
        res = None
        while time.time() - start < self.timeout:
            try:
                res = self.main_controller.recv(flags=zmq.NOBLOCK)
            except zmq.core.error.ZMQError:
                time.sleep(.2)

        if res is None:
            raise TimeoutError()
        return res

    def execute(self, func_name, data, timeout=5.):
        # create a job ID
        short = binascii.b2a_hex(os.urandom(10))[:10]
        job_id = str(int(time.time())) + short
        job = '%s:%s:%s' % (job_id, func_name, data)

        # callback with the result
        _result = []

        def done(job_id, status, data):
            _result.append((status, data))

        self.receiver.register(done, job_id)
        self.sender.send(job)

        # waiting for the result
        start = time.time()

        while time.time() - start < timeout:
            if len(_result) > 0:
                break
            time.sleep(.05)

        if len(_result) == 0:
            raise TimeoutError()

        return _result[0]
