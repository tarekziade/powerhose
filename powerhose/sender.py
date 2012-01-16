import time
import os
import binascii
import zmq
from  multiprocessing import Process
import threading
from collections import defaultdict
from contextlib import contextmanager


class TimeoutError(Exception):
    pass


class Receiver(threading.Thread):
    """Receive results asynchronously
    """
    def __init__(self, receiver):
        threading.Thread.__init__(self)
        self.receiver = receiver
        self.running = False
        self._callbacks = defaultdict(list)

    def stop(self):
        self.running = False
        self.join(timeout=1.)

    def register(self, callback, job_id):
        self._callbacks[job_id].append(callback)

    def run(self):
        self.running = True

        while self.running:
            res = self.receiver.recv()

            job_id, data = res.split(':', 1)

            for callback in self._callbacks[job_id]:
                callback(job_id, data)

            time.sleep(.1)


WORK = 'ipc:///tmp/sender'
RES = 'ipc:///tmp/receiver'
CTR = 'ipc:///tmp/controller'


@contextmanager
def PowerHose():
    sender = Sender()
    try:
        yield sender
    finally:
        sender.stop()


class Sender(object):
    """ Use this class to send jobs.
    """
    def __init__(self):
        # Initialize a zeromq context
        self.context = zmq.Context()
        # Set up a channel to send work
        self.ventilator_send = self.context.socket(zmq.PUSH)
        self.ventilator_send.bind(WORK)
        # Set up a channel to receive results
        self.results_receiver = self.context.socket(zmq.PULL)
        self.results_receiver.bind(RES)
        self.receiver = Receiver(self.results_receiver)
        self.receiver.start()

        # Set up a channel to send control commands
        self.control_sender = self.context.socket(zmq.PUB)
        self.control_sender.bind(CTR)

    def stop(self):
        self.control('FINISH')
        self.control_sender.close()
        self.results_receiver.close()
        self.ventilator_send.close()
        self.receiver.stop()

    def control(self, msg):
        self.control_sender.send(msg)

    def execute(self, func_name, data, timeout=5.):
        # create a job ID
        short = binascii.b2a_hex(os.urandom(10))[:10]
        job_id = str(int(time.time())) + short
        job = '%s:%s:%s' % (job_id, func_name, data)

        # callback with the result
        _result = []

        def done(job_id, data):
            _result.append(data)

        self.receiver.register(done, job_id)
        self.ventilator_send.send(job)

        # waiting for the result
        start = time.time()
        try:
            while time.time() - start < timeout:
                if len(_result) > 0:
                    break
                time.sleep(.1)
        except KeyboardInterrupt:
            pass

        if len(_result) == 0:
            raise TimeoutError()

        return _result[0]
