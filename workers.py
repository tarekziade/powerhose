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
            res = self.receiver.recv_json()
            print 'received ' + str(res)
            self.rcpt[res['id']] = res
            time.sleep(0.2)

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
        # Create a pool of workers to distribute work to
        worker_pool = range(10)
        for wrk_num in range(len(worker_pool)):
            Process(target=worker, args=(wrk_num,)).start()

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


def worker(wrk_num):
    # Initialize a zeromq context
    context = zmq.Context()

    # Set up a channel to receive work from the ventilator
    work_receiver = context.socket(zmq.PULL)
    work_receiver.connect(WORK)

    # Set up a channel to send result of work to the results reporter
    results_sender = context.socket(zmq.PUSH)
    results_sender.connect(RES)

    # Set up a channel to receive control messages over
    control_receiver = context.socket(zmq.SUB)
    control_receiver.connect(CTR)
    control_receiver.setsockopt(zmq.SUBSCRIBE, "")

    # Set up a poller to multiplex the work receiver and control receiver
    # channels
    poller = zmq.Poller()
    poller.register(work_receiver, zmq.POLLIN)
    poller.register(control_receiver, zmq.POLLIN)

    # Loop and accept messages from both channels, acting accordingly
    while True:
        socks = dict(poller.poll())

        # If the message came from work_receiver channel, square the number
        # and send the answer to the results reporter
        if socks.get(work_receiver) == zmq.POLLIN:
            work_message = work_receiver.recv_json()
            product = work_message['num'] * work_message['num']
            answer_message = {'worker' : wrk_num, 'result' : product,
                              'id': work_message['id']}
            print 'job done'
            results_sender.send_json(answer_message)

        # If the message came over the control channel, shut down the worker.
        if socks.get(control_receiver) == zmq.POLLIN:
            control_message = control_receiver.recv()
            if control_message == "FINISHED":
                print("Worker %i received FINSHED, quitting!" % wrk_num)
                break

if __name__ == "__main__":


    # Start the ventilator!
    ventilator = Sender()

    # sending a job
    job = {'num': 4}
    print ventilator.execute(job)

    ventilator.stop()
