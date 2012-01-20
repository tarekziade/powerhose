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

using namespace zmq;
using namespace std;

namespace powerhose
{
  socket_t* sockets[4];
  void (*teardown)(Registry) = NULL;
  Registry registry;

  static const char* const WORK = "ipc:///tmp/sender" ;
  static const char* const RES = "ipc:///tmp/receiver" ;
  static const char* const CTR = "ipc:///tmp/controller" ;
  static const char* const MAIN = "tcp://*:5555" ;


  void bye(int param) {
    // cleanup
    for (int i=0; i<3; i++) {
        if (sockets[i]) {
            sockets[i]->close();
            sockets[i] = NULL;
        }
    }

    // call the tearDown
    if (teardown != NULL) {
        teardown(registry);
    }

    exit(1);
  }


  void str2msg(string* data, message_t* msg) {
    const char* sres = data->c_str();
    msg->rebuild((void *)(sres), data->size(), NULL, NULL);
  }

  string msg2str(message_t* msg) {
        size_t size = msg->size();
        char data[msg->size() + 1];
        memcpy(data, msg->data(), size);
        data[size] = 0;
        string res = data;
        return res;
    }


  void worker(Functions functions, void (*setUp)(Registry), void (*tearDown)(Registry)) {
    teardown = tearDown;

    // call the setUp
    if (setUp != NULL) {
        setUp(registry);
    }

    context_t ctx(1);

    // channel to receive work
    socket_t receiver(ctx, ZMQ_PULL);
    receiver.connect(WORK);
    sockets[0] = &receiver;

    // channel to send back results
    socket_t sender(ctx, ZMQ_PUSH) ;
    sender.connect(RES);
    sockets[1] = &sender;

    // channel for the controller
    socket_t control(ctx, ZMQ_SUB) ;
    control.connect(CTR);
    sockets[2] = &control;

    // Set up a poller to multiplex the work receiver and
    // control receiver channels
    zmq_pollitem_t items[2];
    items[0].socket = receiver;
    items[0].events = ZMQ_POLLIN;
    items[1].socket = control;
    items[1].events = ZMQ_POLLIN;

    // now loop and accept messages from the poller
    while (true) {
        poll(items, 2, -1);

        // getting the receiver jobs
        for (short j = 0; j < items[0].revents; j++) {
            cout << "Worker " << getpid() << " received some work to do" << endl;
            message_t job;
            receiver.recv(&job);

            string sjob = powerhose::msg2str(&job);

            cout << "Received " << sjob.data() << endl;

            // extracting the id, the func name and the job data
            size_t pos = sjob.find(':');
            string job_data = sjob.substr(pos + 1, sjob.size() - pos);
            string job_id = sjob.substr(0, pos);

            pos = job_data.find(':');
            string job_data2 = job_data.substr(pos + 1, job_data.size() - pos);
            string job_func = job_data.substr(0, pos);

            Functions::iterator iter = functions.begin();
            iter = functions.find(job_func);

            string (*function)(string, Registry) = NULL;

            if (iter != functions.end())  {
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
            sender.send(mres);
        }

        // getting the controller jobs
        for (short j = 0; j < items[1].revents; j++) {
            message_t job;
            control.recv(&job);
            string sjob = powerhose::msg2str(&job);

            // do something with the message
            cout << "Received an order " << sjob << endl;
        }
    }

    }

  void free_str(void *data, void *hint) {
      free(data);
  }

  int run_workers(int count, Functions functions, void (*setUp)(Registry), void (*tearDown)(Registry)) {
    signal(SIGINT, bye);
    signal(SIGTERM, bye);

    // setting up the main controller
    context_t ctx(1);

    // channel to receive commands
    socket_t main_controller(ctx, ZMQ_REP);
    main_controller.bind(MAIN);
    sockets[3] = &main_controller;

    cout << "Starting 10 workers." << endl;
    int pids [10];
    string sid;
    short i = 1;

    while (i < count) {
        pid_t pid = fork();
        if (pid == 0) {
            sid = "child";
            pids[i] = pid;
            worker(functions, setUp, tearDown);
            i = count;
        }
        else {
            sid = "parent";
            i++;
        }
    }
    if (sid == "parent") {
        cout << "Listening to commands" << endl;
        // get calls
        while (true) {
            cout << "Waiting" << endl;
            message_t request;
            main_controller.recv(&request);
            string smsg = powerhose::msg2str(&request);

            cout << "Received " << smsg << endl;
            if (smsg == "NUMWORKERS") {
                ostringstream res;
                res << count;
                string sres = res.str();
                message_t reply;
                str2msg(&sres, &reply);
                main_controller.send(reply);
            }
            else if (smsg=="PING") {
                message_t reply(4);
                memcpy(reply.data(), "PONG", reply.size());
                main_controller.send(reply);
            }
            else {
                message_t reply(4);
                memcpy(reply.data(), "NOOP", reply.size());
                main_controller.send(reply);
            }
        }


        // here, loop to wait for all childs to die.

        for (int i = 0; i < count; ++i) {
            int status;
            while (-1 == waitpid(pids[i], &status, 0));
            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                cerr << "Process " << i << " (pid " << pids[i] << ") failed" << endl;
                exit(1);
            }
        }
        cout << "Exiting main " << getpid() << endl;
    }

    return 0;
    }
}
