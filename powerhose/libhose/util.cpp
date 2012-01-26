#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <map>
#include <sstream>
#include "util.h"

using namespace zmq;
using namespace std;


namespace powerhose
{
  void free_str(void *data, void *hint) {
      free(data);
  }

  /*
   * Helpers
   */
  void str2msg(string* data, message_t* msg) {
    const char* sres = data->c_str();
    msg->rebuild((void *)(sres), data->size(), NULL, NULL);
  }

  string msg2str(message_t* msg) {

        size_t size = msg->size();
        char* data = new char[msg->size() + 1];
        memcpy(data, msg->data(), size);
        data[size] = 0;
        string res = data;
        return res;
    }
}
