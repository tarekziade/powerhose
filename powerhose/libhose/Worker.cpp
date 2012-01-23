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


  Worker::Worker(Functions functions, void (*setUp)(Registry), void (*tearDown)(Registry)) {
    this->functions = &functions;
    this->setUp = setUp;
    this->tearDown = tearDown;
    this->ctx = new context_t(1);
    this->receiver = new socket_t(*this->ctx, ZMQ_PULL);
    this->receiver->connect(WORK);
    this->sender = new socket_t(*this->ctx, ZMQ_PUSH);
    this->sender->connect(RES);
    this->control = new socket_t(*this->ctx, ZMQ_SUB);
    this->control->connect(CTR);

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

    cout << "starting a worker" << endl;

    // call the setUp
    if (this->setUp != NULL) {
        cout << "calling setup" << endl;

        this->setUp(registry);
    }


    // now loop and accept messages from the poller
    while (true) {
        cout << "calling poll" << endl;

        poll(this->poll_items, 2, -1);

        cout << "polled called" << endl;

        // getting the receiver jobs
        for (short j = 0; j < this->poll_items[0].revents; j++) {
            cout << "Worker " << getpid() << " received some work to do" << endl;
            message_t job;
            this->receiver->recv(&job);

            string sjob = powerhose::msg2str(&job);

            cout << "Received " << sjob.data() << endl;

            // extracting the id, the func name and the job data
            size_t pos = sjob.find(':');
            string job_data = sjob.substr(pos + 1, sjob.size() - pos);
            string job_id = sjob.substr(0, pos);

            pos = job_data.find(':');
            string job_data2 = job_data.substr(pos + 1, job_data.size() - pos);
            string job_func = job_data.substr(0, pos);

            Functions::iterator iter = this->functions->begin();
            iter = this->functions->find(job_func);

            string (*function)(string, Registry) = NULL;

            if (iter != this->functions->end())  {
                cout << "Value is: " << iter->second << '\n';
                function = iter->second;
            }
            else {
                cout << "Key is not in my_map" << '\n';
                function = NULL;
            }

            string res;
            string status;

            cout << "calling the func " << job_data2 << endl;

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
                res = "The function is unknown";
                status = "KO";
            }

            cout << "res is" << res << endl;

            // send back the result
            string sres = job_id + ":" + status + ":" + res;
            message_t mres(sres.size());
            memcpy((void *)mres.data(), sres.data(), sres.size());
            cout << "sending " << sres << endl;
            this->sender->send(mres);
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
