#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
 
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>

#include <time.h>

#include <sys/time.h>

#define MAXWORKERS 640   /* maximum number of workers */
#define DEFAULTMEN 320
#define MAXFLIPS 100
int numMen, numWomen, numWorkers, numFlips;
double start_time, end_time;
/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

int max(int a, int b){
  if (a>b) 
    return a;
  else
    return b;
} 

void* Worker(void *);

pthread_attr_t attr;


sem_t qacc[2], awake[2], countermutex[2], sleepingmutex, flipmutex;  //initiallize all these
int waiting[2], inbathroom, sleeping;

int main(int argc, char *argv[]) {

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  sem_init(&sleepingmutex, 0, 1);
  sem_init(&flipmutex, 0, 1);

  for(int i = 0; i < 2; i++){
    sem_init(&countermutex[i], 0, 1);
    sem_init(&awake[i], 0, 0); //both can initially add to the queues, but not awake themselves
    sem_init(&qacc[i], 0, 1);
  }

  /* read command line args if any */
  numFlips = (argc > 1)? atoi(argv[1]) : MAXFLIPS;
  numMen = (argc > 2)? atoi(argv[2]) : DEFAULTMEN;
  numWorkers = (argc > 3)? atoi(argv[3]) : MAXWORKERS;

  if (numFlips > MAXFLIPS) numFlips = MAXFLIPS;
  if (numMen > MAXWORKERS) numMen = MAXWORKERS;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  numWomen = numWorkers - numMen;
  assert(numWomen > 0); assert(numMen>0);

  pthread_t workerid[numWorkers];

  start_time = read_timer();
  /* Create all the threads */
  for(int i = 0; i < numWorkers; i++){
    if (i < numMen)
        pthread_create(&workerid[i], &attr, Worker, (void *) 0);
    else
        pthread_create(&workerid[i], &attr, Worker, (void *) 1); 
  }

  //in here, I will start the process, by closing the zero queue, and awaking all the processess waiting to enter
  sem_wait(&qacc[0]);
  sem_post(&awake[0]);

  /*join all the workers, I could assume that they all quit, but it is better to check*/
  for(int i = 0; i< numWorkers; i++){
    pthread_join(workerid[i], NULL);
  }
  /* get end time */
  end_time = read_timer();

  /* print results, what do we print? the no flips? the traces, smhw
  maybe every time we let ppl in we print the time stamp, and which threads were let in? */

  printf("\n");
  printf("The execution time is %g sec\n", end_time - start_time);
   
  pthread_exit(NULL);
}   

bool done(){
    sem_wait(&flipmutex);
    if(numFlips <= 0){
        sem_post(&flipmutex);
        return 1;
    }
    sem_post(&flipmutex);
    return 0;

}
void *Worker(void *arg) {
    bool gender = (bool) arg;

    while(true){ 

        if(done()) break; //everywhere there is this statement, there is a possible deadlock,when the program finishes
 
        {
        sem_wait(&qacc[gender]);  //enter the queue
        sem_post(&qacc[gender]);  
        
        sem_wait(&countermutex[gender]);
        waiting[gender]++;              //increase the nr of processess waiting atm
        sem_post(&countermutex[gender]);
        
        if(done()) break; //make sure we aren't done before taking, this could lead to bad situations
        sem_wait(&awake[gender]);  //wait until you are awaken, this is where you are waiting in the "queue"
        
        }
        //upd counters
        {
        sem_wait(&countermutex[gender]);
        waiting[gender]--; //no longer in queue
        inbathroom++;   //this together with the barrier later actually shows the number of ppl in the bathroom at one time
        sem_post(&countermutex[gender]);
        }

        if(waiting[gender] == 0){ //if you are the last one in the queue, you will release the lock on the queue, allowing new threads to fill in
            //this just prints all the information
            {
                sem_wait(&countermutex[gender]);

                printf("gender: %d, number inside %d\n", gender, inbathroom);
                printf("your gender waiting %d, other gender waiting %d \n", waiting[gender], waiting[1-gender]);
                printf("sleeping %d, those who didn't get into queues %d \n\n", sleeping, numWorkers - sleeping - waiting[gender] - waiting[1-gender] - inbathroom);
                //printf("numflips %d", numFlips);
                sem_post(&countermutex[gender]);
            }
            sem_post(&qacc[gender]); 
        }
        else
            sem_post(&awake[gender]); //this passess the awake signal, but doesnt let any new threads access the queue

        // add this if there is need for waiting inside
       // usleep(1000*rand()%99);


        //this check is to ensure, that all the threads have entered the bethroom, before decrementing and starting to go out

        sem_wait(&qacc[gender]); //this might lead to long wait times, worst case until all the threads outside are n queue
        sem_post(&qacc[gender]); 
        
       //this block hands it over to the other side
       {
        sem_wait(&countermutex[gender]);  
        inbathroom--;
        if(inbathroom == 0){
            //in here we know, all processess that should have, have entered and exited the bathroom
            //now we can wake up the other gender, their qacc is open, but their awake signal is not done
            sem_post(&countermutex[gender]); //release this first to avoid possible deadlocks
            
            sem_wait(&flipmutex);
            numFlips--;             //this is done here, so only one htread does it
            sem_post(&flipmutex);

            if(done()) break;  //check it here so that the counting is accurate

            //wait until the other queue is not empty
            {
            sem_wait(&countermutex[1-gender]);
            while(waiting[1-gender] == 0){//only lock the other queue if there is somebody waiting
                sem_post(&countermutex[1-gender]); 
                usleep(10000);//busywaiting
                sem_wait(&countermutex[1-gender]);
            }
            sem_post(&countermutex[1-gender]); 
            }

            sem_wait(&qacc[1-gender]); //block the otheer gender from joining the queue
            sem_post(&awake[1-gender]); //awake all inthe other queue
        } 
        else
            sem_post(&countermutex[gender]);
    
        }

        //we sleep last cuz otherwise deadlock
        {
        sem_wait(&sleepingmutex);
        sleeping++;
        sem_post(&sleepingmutex);

        usleep(10*(rand()%999)); //lets try a few ms first

        sem_wait(&sleepingmutex);
        sleeping--;
        sem_post(&sleepingmutex);
        }
        
    }

    for(int j = 0; j < 2; j++){
        sem_post(&qacc[j]);
        sem_post(&awake[j]);
    }
    return NULL;
}

