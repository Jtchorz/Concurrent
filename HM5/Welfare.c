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
//all can generate and sort in here as all do it
    data[0] = 0;
    for (int i = 1; i < arr_size; i++) 
        data[i] = data[i - 1] + rand() % 999;     //this creates a sorted array with unique values
    
    
    if(rank == 0 ){
        for(int i = 0; i < arr_size; i++)
            MPI_Send(&data[i], 1, MPI_INT, rank+1, rank, MPI_COMM_WORLD);
    }else if( rank != 3 ){
    //this code is actually scalable amnt of nodes, could be more than 3
    //in here we take in an int from the node before, match to our int, if it doesnt match, throw it away
        int i = 0, possible;
        while(i < arr_size){
            MPI_Recv(&possible, 1, MPI_INT, rank-1, rank-1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
            while(i < arr_size && possible < data[i])
                i++;
            if(possible == data[i])
                MPI_Send(&data[i], 1, MPI_INT, rank+1, rank, MPI_COMM_WORLD);        
        }         
    }else {
       //this is the congregating process, so it will write it's possibles to data and then broadcast
        int i = 0, possible;
        while(i < arr_size){
            MPI_Recv(&possible, 1, MPI_INT, rank-1, rank-1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
            while(i < arr_size && possible < data[i])
                i++;
            if(possible == data[i]){
                result[res_size] = possible;
                res_size++;
            }
        }
    }

    MPI_Bcast(&res_size, 1, MPI_INT, size-1, MPI_COMM_WORLD);

    for(int i = 0; i < res_size; i++){
        MPI_Bcast(&result[i], 1, MPI_INT, size-1, MPI_COMM_WORLD);
    }
    
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