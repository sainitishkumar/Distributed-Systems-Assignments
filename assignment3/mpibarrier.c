/*
*  Implemented using the tree approach
*  discussed during class
*/

#include <stdio.h>
#include <mpi.h>

int parent(int rank)
{
    if(rank%2==0)
        return (rank-2)/2;
    return (rank-1)/2;
}

void mpi_iittp_barrier(int rank, int world_size)
{
    if(world_size==1)
        return;
    if(rank==0)
    {
        int token = 1;
        if(1<world_size)
            MPI_Recv(&token, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(2<world_size)
            MPI_Recv(&token, 1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(1<world_size)
            MPI_Send(&token, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        if(2<world_size)
            MPI_Send(&token, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
    }
    else
    {
        if(2*rank+1 < world_size)
        {
            int token=0;
            int parent1 = parent(rank);
            
            if(2*rank+1 < world_size)
                MPI_Recv(&token, 1, MPI_INT, 2*rank+1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(2*rank+2 < world_size)
                MPI_Recv(&token, 1, MPI_INT, 2*rank+2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&token, 1, MPI_INT, parent1, 1, MPI_COMM_WORLD);

            MPI_Recv(&token, 1, MPI_INT, parent1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(2*rank+1 < world_size)
                MPI_Send(&token, 1, MPI_INT, 2*rank+1, 0, MPI_COMM_WORLD);
            if(2*rank+2 < world_size)
                MPI_Send(&token, 1, MPI_INT, 2*rank+2, 0, MPI_COMM_WORLD);
        }
        else if(2*rank+1 >= world_size) // no children in tree
        {
            int token=0;
            int parent1 = parent(rank);
            MPI_Send(&token, 1, MPI_INT, parent1, 1, MPI_COMM_WORLD);
            MPI_Recv(&token, 1, MPI_INT, parent1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    return;
}

int main(int argc, char const *argv[])
{
    MPI_Init(NULL, NULL);
    //no of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    //rank of each process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // fflush(stdout);
    printf("before %d\n", rank);

    mpi_iittp_barrier(rank, world_size);
    // MPI_Barrier(MPI_COMM_WORLD);
    // fflush(stdout);
    printf("after %d\n", rank);
    
    return 0;
}