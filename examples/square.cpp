#include <string.h>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "job.pb.h"
#include <sys/wait.h>
#include <map>
#include "libhose.h"

using namespace std;
using namespace powerhose;


string square(string job) {
    Job pjob;
    pjob.ParseFromString(job);
    pjob.set_param(pjob.param() * pjob.param());
    string res;
    pjob.SerializeToString(&res);
    return res;
}


int main(int argc, const char* const argv[]) {
  Functions functions;
  functions.insert(Function("square", &square));
  return run_workers(10, functions);
}
