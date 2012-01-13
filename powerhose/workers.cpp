#include <string.h>
#include <zmq.hpp>

using namespace zmq;
using namespace std;

int main(int argc, const char* const argv[])
{
  static const char* const WORK = "ipc:///tmp/sender" ;
  static const char* const RES = "ipc:///tmp/receiver" ;
  static const char* const CTR = "ipc:///tmp/controller" ;

  context_t ctx(1);

  // channel to receive work
  socket_t receiver(ctx, ZMQ_PULL);
  receiver.connect(WORK);

  // channel to send back results
  socket_t sender(ctx, ZMQ_PUSH) ;
  sender.connect(RES);

  // channel for the controller
  socket_t control(ctx, ZMQ_SUB) ;
  control.connect(CTR);

  // Set up a poller to multiplex the work receiver and
  // control receiver channels
  zmq_pollitem_t items[2];
  items[0].socket = receiver;
  items[0].events = ZMQ_POLLIN;
  items[1].socket = control;
  items[1].events = ZMQ_POLLIN;

  // now loop and accept messages from the poller
  while (true) {
    poll(items, 2, -1);

    // getting the receiver jobs
    for (short j = 0; j < items[0].revents; j++) {
        message_t job;
        receiver.recv(&job);
        // do something with the message

        // send back the result
        message_t res;
        memset(res.data(), 0, res.size());
        sender.send(res);

    }

    // getting the controller jobs
    for (short j = 0; j < items[1].revents; j++) {
        message_t job;
        control.recv(&job);
        // do something with the message
    }
  }

  return 0 ;
}

