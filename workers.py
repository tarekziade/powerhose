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
        self.ventilator_send.send_pyobj(job)
        while job_id not in self.results:
            time.sleep(.1)

        res = self.results[job_id]
        del self.results[job_id]
        return res


class Worker(object):

    def __init__(self, id, funcs, size=10):
        self.funcs = funcs
        self.id = id
        self.context = zmq.Context()

        # Set up a channel to receive work from the ventilator
        self.work_receiver = self.context.socket(zmq.PULL)
        self.work_receiver.connect(WORK)

        # Set up a channel to send result of work to the results reporter
        self.results_sender = self.context.socket(zmq.PUSH)
        self.results_sender.connect(RES)

        # Set up a channel to receive control messages over
        self.control_receiver = self.context.socket(zmq.SUB)
        self.control_receiver.connect(CTR)
        self.control_receiver.setsockopt(zmq.SUBSCRIBE, "")

        # Set up a poller to multiplex the work receiver and control receiver
        # channels
        self.poller = zmq.Poller()
        self.poller.register(self.work_receiver, zmq.POLLIN)
        self.poller.register(self.control_receiver, zmq.POLLIN)

        self.start()

    def start(self):
        # Loop and accept messages from both channels, acting accordingly
        while True:
            socks = dict(self.poller.poll())

            # If the message came from work_receiver channel, square the number
            # and send the answer to the results reporter
            if socks.get(self.work_receiver) == zmq.POLLIN:
                work_message = self.work_receiver.recv_pyobj()
                func_id = work_message['func']
                func = self.funcs[func_id]
                del work_message['func']

                data = {
                    'worker': self.id,
                    'result': func(**work_message),
                    'id': work_message['id']}

                self.results_sender.send_pyobj(data)

            # If the message came over the control channel,
            # shut down the worker.
            if socks.get(self.control_receiver) == zmq.POLLIN:
                control_message = self.control_receiver.recv()
                if control_message == "FINISHED":
                    print("Worker %i received FINSHED, quitting!" % wrk_num)
                    break


def create_pool(size, funcs):
    def worker(id, funcs):
        return Worker(id, funcs)

    for id in range(size):
        Process(target=worker, args=(id, funcs)).start()



if __name__ == "__main__":

    def square(num, **kwargs):
        print 'job done'
        return num * num

    funcs = {'square': square}

    # Create a pool of workers to distribute work to
    create_pool(10, funcs)

    # Start the ventilator!
    ventilator = Sender()

    # sending a job
    for i in xrange(1, 10, 4):
        ventilator.execute({'num': i, 'func': 'square'})

    ventilator.stop()
