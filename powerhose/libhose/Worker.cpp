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
#include "util.h"

using namespace zmq;
using namespace std;


namespace powerhose
{
  /*
   * Workers Class
   *
   */

 // see how to use an active worker design when it pings it's not busy

  Worker::Worker(Functions* functions, void (*setUp)(Registry),
		  void (*tearDown)(Registry), const char* senderChannel, 
		  const char* receiverChannel, const char* controllerChannel) {

    this->functions = functions;
    this->setUp = setUp;
    this->tearDown = tearDown;
    this->ctx = new context_t(1);
    this->receiver = new socket_t(*this->ctx, ZMQ_PULL);
    this->receiver->connect(senderChannel);
    this->sender = new socket_t(*this->ctx, ZMQ_PUSH);
    this->sender->connect(receiverChannel);
    this->control = new socket_t(*this->ctx, ZMQ_SUB);
    this->control->connect(controllerChannel);

    zmq_pollitem_t item1;
    item1.socket = *this->receiver;
    item1.events = ZMQ_POLLIN;
    this->poll_items[0] = item1;

    zmq_pollitem_t item2;
    item2.socket = *this->control;
    item2.events = ZMQ_POLLIN;
    this->poll_items[1] = item2;
  }

  Worker::~Worker() {
    this->receiver->close();
    delete this->receiver;
    this->sender->close();
    delete this->sender;
    this->control->close();
    delete this->control;
    delete this->ctx;
  }


  void Worker::run() {
    // call the setUp
    if (this->setUp != NULL) {
        this->setUp(registry);
    }
    // now loop and accept messages from the poller
    while (true) {
        try {
            poll(this->poll_items, 2, -1);
        }
        catch (...) {
            cout << "something went wrong" << endl;
            break;
        }

        // getting the receiver jobs
        for (short j = 0; j < this->poll_items[0].revents; j++) {
            message_t job;
            this->receiver->recv(&job);

            string sjob = powerhose::msg2str(&job);


            // extracting the id, the func name and the job data
            size_t pos = sjob.find(':');
            string job_data = sjob.substr(pos + 1, sjob.size() - pos);
            string job_id = sjob.substr(0, pos);


            pos = job_data.find(':');
            string job_data2 = job_data.substr(pos + 1, job_data.size() - pos);
            string job_func = job_data.substr(0, pos);

            string (*function)(string, Registry) = NULL;

            function = this->functions->find(job_func)->second;

            string res;
            string status;


            if (function) {
                try {
                    res = (*function)(job_data2, registry);
                    status = "OK";
                }
                catch (...) {
                    status = "KO";
                    res = "details about the error?";
                }
            }
            else {
                res = "The function " + job_func + " is unknown";
                status = "KO";
            }


            // send back the result
            string sres = job_id + ":" + status + ":" + res;
            message_t mres(sres.size());
            memcpy((void *)mres.data(), sres.data(), sres.size());

            try {
                this->sender->send(mres, ZMQ_NOBLOCK);
            }
            catch (...) {
                // the sending timed out
                //XXX
            }
        }

        // getting the controller jobs
        for (short j = 0; j < this->poll_items[1].revents; j++) {
            message_t job;
            this->control->recv(&job);
            string sjob = powerhose::msg2str(&job);

            // do something with the message
            cout << "Received an order " << sjob << endl;
        }
    }
  }
}
