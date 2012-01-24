#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <map>
#include <sstream>

#include "libhose.h"
#include "PowerHose.h"

using namespace zmq;
using namespace std;


namespace powerhose
{

  int run_workers(int count, Functions* functions, void (*setUp)(Registry), void (*tearDown)(Registry)) {
    PowerHose powerhose(10, functions, setUp, tearDown);

    powerhose.run();

    return 0;
    }
}
