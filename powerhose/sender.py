import time
import os
import binascii
import zmq
from  multiprocessing import Process
import threading
from collections import defaultdict


class TimeoutError(Exception):
    pass


class Receiver(threading.Thread):
    """Receive results asynchronously
    """
    def __init__(self, context):
        threading.Thread.__init__(self)
        self.receiver = context.socket(zmq.PULL)
        self.receiver.bind(RES)
        self.running = False
        self._callbacks = defaultdict(list)

    def stop(self):
        self.receiver.close()
        self.running = False
        self.join(timeout=1.)

    def register(self, callback, job_id):
        self._callbacks[job_id].append(callback)

    def run(self):
        self.running = True

        while self.running:
            res = self.receiver.recv()

            # the data we get back is composed of 3 fields
            # - a job id
            # - a status (OK or KO)
            # - extra data that can be the result or a traceback
            job_id, data = res.split(':', 1)
            status, data = data.split(':', 1)

            for callback in self._callbacks[job_id]:
                callback(job_id, status, data)

            time.sleep(.1)


WORK = 'ipc:///tmp/sender'
RES = 'ipc:///tmp/receiver'
CTR = 'ipc:///tmp/controller'


class Sender(object):
    """ Use this class to send jobs.
    """
    def __init__(self):
        # Initialize a zeromq context
        self.context = zmq.Context()
        # Set up a channel to send work
        self.sender = self.context.socket(zmq.PUSH)
        self.sender.bind(WORK)
        # Set up a channel to receive results
        self.receiver = Receiver(self.context)
        self.receiver.start()

        # Set up a channel to send control commands
        self.controller = self.context.socket(zmq.PUB)
        self.controller.bind(CTR)

    def stop(self):
        self.control('FINISH')   # not really used yet
        self.controller.close()
        self.sender.close()
        self.receiver.stop()

    def control(self, msg):
        self.controller.send(msg)

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
