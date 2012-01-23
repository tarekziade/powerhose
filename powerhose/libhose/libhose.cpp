#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <map>
#include <sstream>

#include "libhose.h"
#include "Controller.h"
#include "Worker.h"


using namespace zmq;
using namespace std;

int NUMWORKERS = 10;

namespace powerhose
{

  Worker* worker = NULL;
  Controller* controller = NULL;

  void bye_main(int param) {
    cout << "bye main" << endl;
    if (controller != NULL) {
        delete controller;
    }
    exit(1);
  }

  void bye_worker(int param) {
    cout << "bye worker" << endl;
    if (worker != NULL) {
        delete worker;
    }
    exit(1);
  }


  int run_workers(int count, Functions* functions, void (*setUp)(Registry), void (*tearDown)(Registry)) {
    // starting workers
    int pids[count];
    string sid;
    short i = 1;

    while (i < count) {
        pid_t pid = fork();
        if (pid == 0) {
            sid = "child";
            // starting a worker
            cout << "child" << endl;

            signal(SIGINT, bye_worker);
            signal(SIGTERM, bye_worker);
            cout << "create a worker" << endl;

            worker = new Worker(functions, setUp, tearDown);
            cout << "running a worker" << endl;

            worker->run();
            i = count;
        }
        else {
            sid = "parent";
            pids[i] = pid;
            i++;
        }
    }
    if (sid == "parent") {
        cout << "Listening to commands" << endl;

        signal(SIGINT, bye_main);
        signal(SIGTERM, bye_main);

        // setting up the main controller
        cout << "create a controller" << endl;
        controller = new Controller;
        cout << "running it" << endl;
        controller->run(pids, count);

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
