import time
import os
import binascii
import zmq
from  multiprocessing import Process
import threading
from collections import defaultdict
import sys

from powerhose.soaker import Soaker


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
        self.poll = zmq.Poller()
        self.poll.register(self.receiver, zmq.POLLIN)
        self._callbacks = defaultdict(list)

    def stop(self):
        self.running = False
        time.sleep(.1)
        self.receiver.close()
        #self.join(timeout=1.)

    def register(self, callback, job_id, event=None):
        self._callbacks[job_id].append((callback, event))

    def run(self):
        self.running = True

        while self.running:
            self.poll.poll(self.timeout)

            res = self.receiver.recv()

            # the data we get back is composed of 3 fields
            # - a job id
            # - a status (OK or KO)
            # - extra data that can be the result or a traceback
            job_id, data = res.split(':', 1)
            status, data = data.split(':', 1)

            # the callbacks are called with another thread.
            for callback, event in self._callbacks[job_id]:
                try:
                    callback(job_id, status, data)
                except Exception, e:
                    # log this
                    pass
                if event is not None:
                    event.set()

            # we're done here
            del self._callbacks[job_id]


WORK = 'ipc:///tmp/sender'
RES = 'ipc:///tmp/receiver'
CTR = 'ipc:///tmp/controller'
MAIN = 'tcp://*:5555'


class Sender(object):
    """ Use this class to send jobs.
    """
    def __init__(self, timeout=5.):
        self.timeout = timeout

        # Initialize a zeromq context
        self.context = zmq.Context()

        # set up the main controller
        self.soaker = Soaker(MAIN, timeout)

        # Set up a channel to receive results
        self.receiver = Receiver(self.context, self.timeout)
        self.receiver.start()

        # Set up a channel to send work
        self.sender = self.context.socket(zmq.PUSH)
        self.sender.bind(WORK)

    def stop(self):
        self.soaker.stop()
        self.sender.close()
        self.receiver.stop()
        self.context.destroy(0)

    def execute(self, func_name, data, timeout=5.):
        # create a job ID
        short = binascii.b2a_hex(os.urandom(10))[:10]
        job_id = str(int(time.time())) + short
        job = '%s:%s:%s' % (job_id, func_name, data)

        # callback with the result
        _result = []

        def done(job_id, status, data):
            _result.append((status, data))

        # XXX can we share the event ? I don't think so..
        lock = threading.Event()
        self.receiver.register(done, job_id, lock)
        self.sender.send(job)

        # waiting for the result
        lock.wait(self.timeout)

        if len(_result) == 0:
            raise TimeoutError()

        return _result[0]
