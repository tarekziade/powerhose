#ifndef WORKER_H
#define WORKER_H

#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <map>
#include <sstream>

#include "util.h"

using namespace zmq;
using namespace std;


namespace powerhose
{
  class Worker {

    private:
        context_t* ctx;
        socket_t* receiver;
        socket_t* sender;
        socket_t* control;
        zmq_pollitem_t poll_items[2];
        Functions *functions;
        void (*setUp)(Registry);
        void (*tearDown)(Registry);
        Registry registry;
    public:
       Worker(Functions* functions, void (*setUp)(Registry),
			  void (*tearDown)(Registry), const char* senderChannel,
			  const char* receiverChannel, const char* controllerChannel);
       ~Worker();
       void run();
  };

}

#endif
