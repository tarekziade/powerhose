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
    pjob.set_value(pjob.value() * pjob.value());
    string res;
    pjob.SerializeToString(&res);
    return res;
}


int main(int argc, const char* const argv[]) {
  int num = 10;

  // building the map of functions
  Function fsquare = Function("square", &square);
  Functions functions;
  functions.insert(fsquare);

  // running the workers
  return run_workers(num, functions);
}
