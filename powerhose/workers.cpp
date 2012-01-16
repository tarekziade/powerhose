#include <string.h>
#include <zmq.hpp>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "job.pb.h"
#include <sys/wait.h>


using namespace zmq;
using namespace std;


void bye(int param) {
  cout << "Bye" << endl;
  // cleanup ?
  exit(1);
}

string msg2str(message_t* msg) {
    // XXX how to copy directly into the string ?
    size_t size = msg->size();
    char data[msg->size() + 1];
    memcpy(data, msg->data(), size);
    data[size] = 0;
    string res = data;
    return res;
}


void worker() {
  static const char* const WORK = "ipc:///tmp/sender" ;
  static const char* const RES = "ipc:///tmp/receiver" ;
  static const char* const CTR = "ipc:///tmp/controller" ;

  context_t ctx(1);

  // channel to receive work
  socket_t receiver(ctx, ZMQ_PULL);
  receiver.connect(WORK);

  // channel to send back results
  socket_t sender(ctx, ZMQ_PUSH) ;
  sender.connect(RES);

  // channel for the controller
  socket_t control(ctx, ZMQ_SUB) ;
  control.connect(CTR);

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

        string sjob = msg2str(&job);

        cout << "Received " << sjob.data() << endl;

        // extracting the id and the job data
        size_t pos = sjob.find(':');
        string job_data = sjob.substr(pos + 1, sjob.size() - pos);
        string job_id = sjob.substr(0, pos);

        Job pjob;
        pjob.ParseFromString(job_data);

        cout << "Job Id " << job_id << endl;
        cout << "Job func " << pjob.func() << endl;
        cout << "Job param " << pjob.param() << endl;

        // send back the result
        //message_t res(jpb.sized());
        //memcpy((void *) res.data (), sjob, job.size());
        string sres = job_id + ":" + sjob.data();
        message_t res(sres.size());
        memcpy((void *)res.data(), sres.data(), sres.size());
        cout << "sending " << sres << endl;
        sender.send(res);
    }

    // getting the controller jobs
    for (short j = 0; j < items[1].revents; j++) {
        message_t job;
        control.recv(&job);
        // do something with the message
    }
  }

}


int main(int argc, const char* const argv[]) {
  signal(SIGINT, bye);
  int workers_count = 10;
  int pids [10];
  string sid;
  short i = 1;

  while (i < workers_count) {
    pid_t pid = fork();
    if (pid == 0) {
        sid = "child";
        pids[i] = pid;
        cout << "Starting worker " << getpid() << endl;
        worker();
        i = workers_count;
    }
    else {
        sid = "parent";
        i++;
    }
  }
  if (sid == "parent") {
     // here, loop to wait for all childs to die.
     for (int i = 0; i < workers_count; ++i) {
        int status;
        while (-1 == waitpid(pids[i], &status, 0));
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            cerr << "Process " << i << " (pid " << pids[i] << ") failed" << endl;
            exit(1);
        }
     }
  }
  return 0;
}
