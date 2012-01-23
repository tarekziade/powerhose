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
  void free_str(void *data, void *hint);
  void str2msg(string* data, message_t* msg);
  string msg2str(message_t* msg);
}

