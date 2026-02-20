#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <sys/time.h>
#define MAXSIZE 1000000 /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

int numWorkers = 0;           /* number of workers */ 

int thread_num = 1;
pthread_mutex_t mutex_thread;

struct pointer_with_size{
  int* pointer;
  int size;
};

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
int size;  
int array[MAXSIZE]; /* matrix */
void *Worker(void *);
pthread_attr_t attr;

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i;

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
          array[i]=/* 1;*/ rand()%99;
  }

  /* print the array */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	    printf("%d ", array[i]);
	  }
	  printf("\n");
#endif

  //make the first struct to pass

  struct pointer_with_size original;
  original.pointer = array;
  original.size = size;

  /* do the parallel work: create the workers */
  start_time = read_timer();

  //to not leave the main thread just waiting around, I am calling this function instead of creating a worker
  // This makes no threads idly waiting
  
  Worker((void *) &original);

  /* get end time */
  end_time = read_timer();

  /* print results */
  printf("The sorted array is: \n");
for(i = 0; i<size; i++)
    printf("%d ", array[i]);

  printf("\n");
  printf("The execution time is %g sec\n", end_time - start_time);
   
  pthread_exit(NULL);
}   





/*each worker can create a thread, but is setup in such a way to never wait ildly*/
void *Worker(void *arg) {
  int* my_array = ((struct pointer_with_size *)arg) -> pointer;
  int my_size =   ((struct pointer_with_size *)arg) -> size;
  int i, pivot, low, high, temp, new_thread;
  pthread_t thread_pid;

  //if size = 1 return
  if(my_size <= 1){
    return NULL;
  }


  //first, get a pivot
  pivot = my_array[my_size-1];
  //declare indexes of the arrays
  low = 0;
  high = my_size-2;

  //then exchange places of elements until there are two partitions
  while(low <= high){
    if(my_array[high] < pivot && my_array[low] >= pivot) {
      temp = my_array[high];
      my_array[high] = my_array[low];
      my_array[low] = temp;

      high--;
      low++;
    }
    if(my_array[high] >= pivot) high--;
    if(my_array[low] < pivot) low++;
  }
  //low is always safe, it always point either to pivot, or last known value bigger than pivot
  //high can be -1
  my_array[my_size-1] = my_array[low]; 
  my_array[low] = pivot;

  #ifdef DEBUG
  for (i = 0; i < my_size; i++) {
    printf("%d ", my_array[i]);
  }
  printf("\n");
  printf("low: %d high: %d pivot: %d\n", low, high, pivot);
  #endif  

  //pivot is kept back, otherwise infinite loops
  //set up the structs for both halves

  struct pointer_with_size lower, upper;
  lower.pointer = my_array;
  lower.size = low;

  upper.pointer = (my_array+low+1);
  upper.size = my_size-(low+1);

  #ifdef DEBUG
  printf("smaller: ");
  for (i = 0; i < lower.size; i++) {
    printf("%d ", lower.pointer[i]);
  }
  printf("\n");
    printf("bigger: ");
  for (i = 0; i < upper.size; i++) {
    printf("%d ", upper.pointer[i]);
  }
   printf("\n");
  #endif  


  
  new_thread = 0;  
  //check if we have a worker spot to run the bigger half in a new thread
  //only try to parallelize if our array is above the whole data /20, otherwise kinda wasting resources
  //this really boosts performance. shows that in this task maybe a pool of workers could be better,
  if(my_size > size / 20){
    pthread_mutex_lock(&mutex_thread);
    if (thread_num < numWorkers){
      thread_num++;
      //tell current thread that there is a spot
      new_thread = 1;
    }
    pthread_mutex_unlock(&mutex_thread);
  }

  if(new_thread){
    //create a thread to handle the lower part of the array
    //delegate the bigger half to the thread, to make parallelism and thread creation worth it
    if(lower.size > upper.size){
      pthread_create(&thread_pid, &attr, Worker, (void *) &lower);
      Worker((void *) &upper); //work on the other one normally, no need to sit ildly
    }
    else{
      pthread_create(&thread_pid, &attr, Worker, (void *) &upper);
      Worker((void *) &lower);
    }
  }
  else{
    Worker((void *) &lower);  
    Worker((void *) &upper);
  }

  //wait for the thread to return if it was parrallelized
  if(new_thread){
    pthread_join(thread_pid, NULL);
    pthread_mutex_lock(&mutex_thread);
    thread_num--;
    pthread_mutex_unlock(&mutex_thread);
  }

  //othervise return
}





