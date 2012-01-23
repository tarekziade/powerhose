#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <map>
#include <sstream>

#include <libhose.h>
#include "Controller.h"

using namespace zmq;
using namespace std;


namespace powerhose
{

  Controller::Controller() {
    this->ctx = new context_t(1);
    this->socket = new socket_t(*this->ctx, ZMQ_REP);
    this->socket->bind(MAIN);
  }

  Controller::~Controller() {
    this->socket->close();
    delete this->socket;
    delete this->ctx;
  }

  void Controller::run(int pids[], int count) {

    while (true) {
        cout << "Waiting" << endl;
        message_t request;
        this->socket->recv(&request);
        string smsg = powerhose::msg2str(&request);
        cout << "Received " << smsg << endl;

        if (smsg == "NUMWORKERS") {
            ostringstream res;
            res << count;
            string sres = res.str();
            message_t reply;
            str2msg(&sres, &reply);
            this->socket->send(reply);
        }
        else if (smsg=="PING") {
            message_t reply(4);
            memcpy(reply.data(), "PONG", reply.size());
            this->socket->send(reply);
        }
        else {
            message_t reply(4);
            memcpy(reply.data(), "NOOP", reply.size());
            this->socket->send(reply);
        }
    }
  }
}
