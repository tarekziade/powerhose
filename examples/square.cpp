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


string square(string job, Registry registry) {
    Job pjob;
    pjob.ParseFromString(job);
    pjob.set_value(pjob.value() * pjob.value());
    string res;
    pjob.SerializeToString(&res);
    return res;
}


int main(int argc, const char* const argv[]) {
  // get the number of workers from the command line
  int num;
  if (argc > 1){
      num = atoi(argv[1]);
  } else {
      num = 10; // default value
  }
  cout << "running the square exemple with " << num << " workers" << endl;

  // building the map of functions
  Function fsquare = Function("square", &square);
  Functions functions;
  functions.insert(fsquare);

  // running the workers
  return run_workers(num, &functions, NULL, NULL);
}
