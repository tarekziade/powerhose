import time
import zmq
from  multiprocessing import Process
import threading

_NUM = 0


def _jobnum():
    global _NUM
    _NUM += 1
    return _NUM


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
            res = self.receiver.recv_pyobj()
            print 'received ' + str(res)
            self.rcpt[res['id']] = res

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
        # store the reference of the function we want to use for our jobs

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
        job_id = _jobnum()
        job['id'] = job_id
        self.ventilator_send.send_json(job)
        while job_id not in self.results:
            time.sleep(.1)

        res = self.results[job_id]
        del self.results[job_id]
        return res


