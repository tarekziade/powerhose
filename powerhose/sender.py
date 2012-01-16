import time
import os
import binascii
import zmq
from  multiprocessing import Process
import threading


class Receiver(threading.Thread):

    def __init__(self, receiver, rcpt):
        threading.Thread.__init__(self)
        self.receiver = receiver
        self.rcpt = rcpt
        self.running = False

    def stop(self):
        self.running = False
        self.join()

    def run(self):
        self.running = True
        print 'Waiting for results'
        while self.running:
            res = self.receiver.recv()
            print 'received ' + res
            job_id, data = res.split(':', 1)
            print 'job id ' + job_id
            self.rcpt[job_id] = data
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
        self.ventilator_send = self.context.socket(zmq.PUSH)
        self.ventilator_send.bind(WORK)
        # Set up a channel to receive results
        self.results_receiver = self.context.socket(zmq.PULL)
        self.results_receiver.bind(RES)
        self.results = {}
        self.receiver = Receiver(self.results_receiver, self.results)
        self.receiver.start()

        # Set up a channel to send control commands
        self.control_sender = self.context.socket(zmq.PUB)
        self.control_sender.bind(CTR)

    def stop(self):
        self.control('FINISH')
        self.receiver.stop()

    def control(self, msg):
        self.control_sender.send(msg)

    def execute(self, job):
        # XXX timeout ? , async ?
        #
        # create a job ID
        short = binascii.b2a_hex(os.urandom(10))[:10]
        job_id = str(int(time.time())) + short
        job = '%s:%s' % (job_id, job)
        self.ventilator_send.send(job)

        print 'sent ' + job_id

        # XXX replace by a signal
        while job_id not in self.results:
            time.sleep(.1)

        print 'get it'
        res = self.results[job_id]
        del self.results[job_id]
        return res
