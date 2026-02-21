#include <mpi.h>   
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define MAXSIZE 100000

int data[MAXSIZE];
int result[MAXSIZE];

int main(int argc, char *argv[]){
    int rank, size, res_size = 0; 
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    assert(size > 2);

    int arr_size;

    arr_size = (argc > 1)? atoi(argv[1]) : MAXSIZE;

    if(arr_size > MAXSIZE)
        arr_size = MAXSIZE;

    srand(time(NULL)+rank);

    //so all three processess have to have different code, first one just sends 
    //second one just recieves, and third one broadcasts the results
    //all first sort, then 2nd  

    printf("Hello, The world has size %d and I have rank %d \n", size, rank);
    data[0] = 0;
    for (int i = 1; i < arr_size; i++) {
        data[i] = data[i - 1] + (rand() % 99);     //this creates a sorted array with unique values
    }
    if(rank == 0 ){
        //the first process just sends, and doesn't recieve
        for(int i = 0; i < arr_size; i++)
            MPI_Send(&data[i], 1, MPI_INT, rank+1, rank, MPI_COMM_WORLD);
        //send a sentinel so the next process knows we are done
        int temp = -1;
        MPI_Send(&temp, 1, MPI_INT, rank+1, rank, MPI_COMM_WORLD);
        
    }else if( rank != size -1 ){
    //this code is actually scalable amnt of nodes, could be more than 3
        //the idea is that all our numbers are sorted and all the ones coming in are sorted
        //we look for the match, discarding any smaller than the one sent in
        //the match is passed along to the next process
        int i = 0, possible = 0;
        while(i < arr_size && possible != -1){
            MPI_Recv(&possible, 1, MPI_INT, rank-1, rank-1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 

            while(i < arr_size && possible > data[i])
                i++;

            if(possible == data[i])
                MPI_Send(&data[i], 1, MPI_INT, rank+1, rank, MPI_COMM_WORLD);       
        } 
        //send a sentinel to tell we are not sending any more
        int temp = -1;
        //recieve and discard any more data as it doesn't match our anymore
        //but the other program is still sending blocking so we just recieve it
        //another sol would be for the programs to communicate back that it finished
        MPI_Send(&temp, 1, MPI_INT, rank+1, rank, MPI_COMM_WORLD);
        while(possible != -1) 
            MPI_Recv(&possible, 1, MPI_INT, rank-1, rank-1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
    }else {
       //this is the congregating process, so it will write it's possibles to data and then be the root for broadcast
        int i = 0, possible = 0;
        while(i < arr_size && possible != -1){
            MPI_Recv(&possible, 1, MPI_INT, rank-1, rank-1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 

            while(i < arr_size && possible > data[i])
                i++;

            if(possible == data[i]){
                result[res_size] = possible;
                res_size++;
            }
        }
        //also recieve and discard all sends from prev, but don't send a sentinel because nobody is listening
        while(possible != -1) 
            MPI_Recv(&possible, 1, MPI_INT, rank-1, rank-1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
    }
    //Broadcast how many we found
    MPI_Bcast(&res_size, 1, MPI_INT, size-1, MPI_COMM_WORLD);
    //bBroadcast the result one by one
    for(int i = 0; i < res_size; i++){
        MPI_Bcast(&result[i], 1, MPI_INT, size-1, MPI_COMM_WORLD);
    }
    //each one prints their respective result
    for(int i = 0; i < size; i++){
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank == i){
            printf("this is rank %d and the array is: ", i); 
            for(int j = 0; j<res_size; j++)
                printf("%d ", result[j]);
            printf("\n");
        }
    }

    MPI_Finalize();
    
    return 0;
}