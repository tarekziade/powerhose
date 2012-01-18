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
    void bye(int param);
    string msg2str(message_t* msg);
    void str2msg(string* data, message_t* msg);
    void worker(Functions functions, void (*setUp)(Registry), void (*tearDown)(Registry));
    extern int run_workers(int count, Functions functions, void (*setUp)(Registry), void (*tearDown)(Registry));
}

