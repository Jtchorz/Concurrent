#include <mpi.h>   
#include <stdio.h>

#define MAXSIZE 100000

int main(int argc, char *argv[]){
    int arr_size;

    arr_size = (argc > 1)? atoi(argv[1]) : MAXSIZE;

    if(arr_size > MAXSIZE)
        arr_size = MAXSIZE;

    int rank, size; 
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    assert(size < 2);
    
    //so all three processess have to have different code, first one just sends 
    //second one just recieves, and third one broadcasts the results
    //all first sort, then 2nd 
//all can generate and sort in here as all do it
    if(rank == 0 ){

    }else if( rank != 3 ){
    //this code is actually scalable amnt of nodes, could be more than 3        
    }else {
       //code for no 3 
    }

//do the barrier and the broadcast here, as all will do the same thing
    MPI_Finalize();
    
    return 0;
}