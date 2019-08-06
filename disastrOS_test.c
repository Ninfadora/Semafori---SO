#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"


// we need this to handle the sleep state

// termina quando il processo non ha più figli?? (Vedere disastrOS_wait)
  void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    //disastrOS_printStatus();
  }
}

void accedi_risorsa(){
	printf("Semaforo del processo %d ha accesso alla risorsa\n",(disastrOS_getpid()));
	disastrOS_printStatus();
	disastrOS_sleep((20-disastrOS_getpid())*2);
	disastrOS_printStatus();
	printf("Semaforo del processo %d lascia la risorsa\n",(disastrOS_getpid()));
}

void childFunction(void* args){
  printf("\n\nHELLO, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=0;
  int mode=0;
  int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
  printf("fd=%d\n", fd);
  printf("PID: %d, terminating\n", disastrOS_getpid());

  for (int i=0; i<(disastrOS_getpid()+1); ++i){
    printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
    //disastrOS_sleep((20-disastrOS_getpid())*5);
  }
  
  disastrOS_printStatus();
  
  int sem = disastrOS_semopen(1, 0);
  printf("Inizio la prima semWait su sem %d, fd:%d \n\n", (disastrOS_getpid())+1, sem );
  disastrOS_semwait(sem);
  
  accedi_risorsa();
  
  printf("Inizio SEMPOST\n\n");
  disastrOS_sempost(sem);
  
  disastrOS_printStatus();
  
  
  if(disastrOS_semclose(sem) == 0){
	  printf("Semaforo chiuso \n\n");
  }
  else{
	  printf("Il semaforo non è chiuso\n\n");
  }
  
  disastrOS_exit(disastrOS_getpid()+1);
  
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started con pid %d\n",disastrOS_getpid());
  disastrOS_spawn(sleeperFunction, 0);
  

  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  for (int i=0; i<4; ++i) {
    int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    int fd=disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd);
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  //disastrOS_printStatus();
  int retval;
  int pid;

  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }
  printf("shutdown!\n");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p\n", childFunction);
  // spawn an init process
  printf("start \n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
