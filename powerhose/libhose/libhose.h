#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <map>

using namespace zmq;
using namespace std;

namespace powerhose {
    typedef map<string, string> Registry;
    typedef map<string, string(*)(string, Registry)> Functions;
    typedef pair<string, string(*)(string, Registry)> Function;
    extern int run_workers(int count, Functions* functions, void (*setUp)(Registry), void (*tearDown)(Registry));
}

