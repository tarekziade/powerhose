#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <map>
#include <sstream>

#include <libhose.h>

using namespace zmq;
using namespace std;


namespace powerhose
{
  static const char* const WORK = "ipc:///tmp/sender" ;
  static const char* const RES = "ipc:///tmp/receiver" ;
  static const char* const CTR = "ipc:///tmp/controller" ;

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
       Worker(Functions functions, void (*setUp)(Registry), void (*tearDown)(Registry));
       ~Worker();
       void run();
  };

}
