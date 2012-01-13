import time
import zmq
from  multiprocessing import Process
import threading


WORK = 'ipc:///tmp/sender'
RES = 'ipc:///tmp/receiver'
CTR = 'ipc:///tmp/controller'


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
                work_message = self.work_receiver.recv_json()
                func_id = work_message['func']
                func = self.funcs[func_id]
                del work_message['func']

                data = {
                    'worker': self.id,
                    'result': func(**work_message),
                    'id': work_message['id']}

                self.results_sender.send_json(data)

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



