/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <sys/time.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

int numWorkers;           /* number of workers */ 


int global_total = 0;   
int global_min = 99999;
int global_max = 0;
pthread_mutex_t mutex_calc;   //one lock for all the calculations when a thread finishes

int min(int a, int b){
  if (a<b) 
    return a;
  else
    return b;
}  
int max(int a, int b){
  if (a>b) 
    return a;
  else
    return b;
} 

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

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

/*initialize lock for updating variables after the thread finishes it's job*/
  pthread_mutex_init(&mutex_calc, NULL);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  assert(size%numWorkers == 0); //this checks if size is multiple of numWorkers 
  stripSize = size/numWorkers;

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] =/* 1;*/ rand()%99;
	  }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);

  //by having to join them all the main thread waits for all to finish
  for (l = 0; l < numWorkers; l++)
    pthread_join(workerid[l], NULL); //by having to join them all the main thread waits for finish

  /* get end time */
  end_time = read_timer();

  /* print results */
  printf("The total is %d\n", global_total);
  printf("The max is %d\n", global_max);
  printf("The min is %d\n", global_min);
  printf("The execution time is %g sec\n", end_time - start_time);
   
  pthread_exit(NULL);
}   

/* Each worker calcs the values in the matrix and then updates the globals,
 after all are joined, we proceed to print */
void *Worker(void *arg) {
  long myid = (long) arg;
  int i, j, first, last, current, total, l_min, l_max;

#ifdef DEBUG
  printf("worker %ld (pthread id %ld) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

   total = 0; l_min = 999; l_max = 0;
    for (i = first; i <= last; i++)
      for (j = 0; j < size; j++)
        {
        current = matrix[i][j]; 
        total += current;
        l_min = min(l_min, current);
        l_max = max(l_max, current); 
        }
        
      //only one lock needed, as all variables are updated at the same time
      pthread_mutex_lock(&mutex_calc);
      global_total += total;
      global_min= min(global_min, l_min);
      global_max = max(global_max, l_max);
      pthread_mutex_unlock(&mutex_calc);
    
}
