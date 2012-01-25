#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <map>

#include "PowerHose.h"
#include "Worker.h"
#include "util.h"


using namespace zmq;
using namespace std;

namespace powerhose {
    static const char* const DEFAULT_PREFIX = "ipc:///tmp/";
    extern int run_workers(const char* identifier, int count, Functions* functions, void (*setUp)(Registry)=NULL, void (*tearDown)(Registry)=NULL, const char* channelPrefix=DEFAULT_PREFIX);
}

