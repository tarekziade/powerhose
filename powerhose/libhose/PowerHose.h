#ifndef POWERHOSE_H
#define POWERHOSE_H

#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <map>
#include <sstream>

#include "Worker.h"
#include "Controller.h"


using namespace zmq;
using namespace std;


namespace powerhose
{
  
  void bye_main(int param);
     void bye_worker(int param);
     void bye_children(int param);
     void child_dies(int param);

  class PowerHose {

  public:
     int numworkers;
     int *pids;
     Worker* worker;
     Controller* controller;
     bool dying;
     Functions *functions;
     void (*setUp)(Registry);
     void (*tearDown)(Registry);
     Registry registry;
     void waitchildren();
     void killchildren();
     PowerHose(int numworkers, Functions* functions, void (*setUp)(Registry), void (*tearDown)(Registry));
     ~PowerHose();
     void run();
  };

}

#endif
