#ifndef CONTROLLER_H
#define CONTROLLER_H


#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <map>
#include <sstream>

using namespace zmq;
using namespace std;


namespace powerhose
{

  static const char* const MAIN = "tcp://*:5555" ;

  class Controller {
      private:
          context_t *ctx;
          socket_t *socket;
      public:
          Controller();
         ~Controller();
         void run(int pids[], int count);
  };



}

#endif
