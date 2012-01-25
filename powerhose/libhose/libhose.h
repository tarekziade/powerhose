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
    extern int run_workers(int count, Functions* functions, void (*setUp)(Registry), void (*tearDown)(Registry));
}

