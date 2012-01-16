#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <map>


using namespace zmq;
using namespace std;

namespace powerhose {
    typedef map<string, string(*)(string)> Functions;
    typedef pair<string, string (*)(string)> Function;
    void bye(int param) ;
    string msg2str(message_t* msg) ;
    void worker(Functions functions) ;
    int run_workers(int count, Functions functions);
}

