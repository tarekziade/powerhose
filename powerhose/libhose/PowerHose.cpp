#include "PowerHose.h"

using namespace zmq;
using namespace std;


namespace powerhose
{
    PowerHose* singleton = NULL;
    Controller* controller = NULL;
    Worker* worker = NULL;

    class SingletonException: public exception
    {
     virtual const char* what() const throw() {
        return "You can't have two instances";
     }
    } singletonex;

  void bye_main(int param) {
    singleton->dying = true;
    singleton->killchildren();
    exit(1);
  }

  void child_dies(int param) {
   if (singleton->dying) {
       return;
   }
   pid_t pid = wait(NULL);
   cout << "respawning a new worker " << endl;
  }

  void bye_worker(int param) {
    singleton->dying = true;
    exit(1);
  }

  PowerHose::PowerHose(const char* identifier, int numworkers,
		  Functions* functions,
          void (*setUp)(Registry),
          void (*tearDown)(Registry),
          const char* channelPrefix) {

    if (singleton != NULL) {
        throw singletonex;
    }
    singleton = this;
    this->numworkers = numworkers;
    this->pids = new int[numworkers];
    this->setUp = setUp;
    this->tearDown = tearDown;
    this->functions = functions;
    this->dying = false;

    // starting workers 
    string sid;
    short i = 1;
    int pgid = getpgid(getpid());

    // get the channels location from the identifier
    string senderChannel, receiverChannel, controllerChannel;

    senderChannel = (string) channelPrefix + (string) identifier + "-sender";
    receiverChannel = (string) channelPrefix + (string) identifier + "-receiver";
    controllerChannel = (string) channelPrefix + (string) identifier + "-controller";

    while (i < this->numworkers) {
        pid_t pid = fork();
        if (pid == 0) {
            sid = "child";
            setpgid(0, pgid);
            // starting a worker
            signal(SIGINT, bye_worker);
            signal(SIGTERM, bye_worker);

            worker = new Worker(functions, setUp, tearDown,
                    senderChannel.c_str(), receiverChannel.c_str(),
                    controllerChannel.c_str());

            worker->run();
            i = this->numworkers;
        }
        else {
            sid = "parent";
            this->pids[i] = pid;
            i++;
            setpgid(pid, pgid);
        }
    }
    if (sid == "parent") {
        signal(SIGINT, bye_main);
        signal(SIGTERM, bye_main);
        signal(SIGCHLD, child_dies);
        controller = new Controller;
    }
  }

  PowerHose::~PowerHose() {
    delete this->pids;

    if (controller != NULL) {
        delete controller;
    }
    if (worker != NULL) {
        delete worker;
    }

  }

  void PowerHose::run() {
      controller->run(this->pids, this->numworkers);
      this->waitchildren();
  }

  void PowerHose::waitchildren() {
    // here, loop to wait for all childs to die.
    for (int i = 0; i < this->numworkers; ++i) {
       int status;
       while (-1 == waitpid(this->pids[i], &status, 0));
       if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
           cerr << "Process " << i << " (pid " << this->pids[i] << ") failed" << endl;
           //exit(1);
      }
    }
  }

  void PowerHose::killchildren() {
    // loop to kill all of them
    for (int i = 0; i < this->numworkers; ++i) {
       kill(this->pids[i], SIGTERM);
    }
  }


}

