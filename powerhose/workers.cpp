#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "job.pb.h"


using namespace zmq;
using namespace std;

void bye(int param) {
  cout << "Bye" << endl;
  // cleanup ?
  exit(1);
}


int main(int argc, const char* const argv[])
{
  signal(SIGINT, bye);
  static const char* const WORK = "ipc:///tmp/sender" ;
  static const char* const RES = "ipc:///tmp/receiver" ;
  static const char* const CTR = "ipc:///tmp/controller" ;

  cout << "Building a context" << endl;
  context_t ctx(1);

  // channel to receive work
  cout << "Starting the receiver channel" << endl;
  socket_t receiver(ctx, ZMQ_PULL);
  receiver.connect(WORK);

  // channel to send back results
  cout << "Starting the result channel" << endl;
  socket_t sender(ctx, ZMQ_PUSH) ;
  sender.connect(RES);

  // channel for the controller
  cout << "Starting the control channel" << endl;
  socket_t control(ctx, ZMQ_SUB) ;
  control.connect(CTR);

  // Set up a poller to multiplex the work receiver and
  // control receiver channels
  cout << "Starting the poller" << endl;
  zmq_pollitem_t items[2];
  items[0].socket = receiver;
  items[0].events = ZMQ_POLLIN;
  items[1].socket = control;
  items[1].events = ZMQ_POLLIN;

  // now loop and accept messages from the poller
  cout << "Looping now..." << endl;

  while (true) {
    poll(items, 2, -1);

    // getting the receiver jobs
    for (short j = 0; j < items[0].revents; j++) {
        cout << "received some work to do" << endl;

        message_t job;
        receiver.recv(&job);

        // XXX why do I have to convert to a string again ?
        // can't I pass it directly to  protobuf ?
        char sjob[job.size()];
        memcpy(sjob, job.data(), job.size());

        Job pjob;
        pjob.ParseFromString(sjob);

        cout << "Job Id " << pjob.id() << endl;
        cout << "Job func " << pjob.func() << endl;
        cout << "Job param " << pjob.param() << endl;

        // send back the result
        message_t res(5);
        memcpy((void *) res.data (), "World", 5);
        sender.send(res);

    }

    // getting the controller jobs
    for (short j = 0; j < items[1].revents; j++) {
        message_t job;
        control.recv(&job);
        // do something with the message
    }
  }
  return 0;
}
