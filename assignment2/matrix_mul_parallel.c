#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    srand(11);

    int n, m, p; // size of arr 1 (n x m) arr2 (m x p)
    n = atoi(argv[1]);
    m = atoi(argv[2]);
    p = atoi(argv[3]);

    int **A = (int **)malloc(n * sizeof(int *));
    for(int i=0;i<n;i++)
        A[i] = (int *)malloc(m * sizeof(int));
    
    int **B = (int **)malloc(m * sizeof(int *));
    for(int i=0;i<m;i++)
        B[i] = (int *)malloc(p * sizeof(int));

    int **C = (int **)malloc(n * sizeof(int *));
    for(int i=0;i<n;i++)
        C[i] = (int *)malloc(p * sizeof(int));
    
    for(int i=0;i<n;i++)
        for(int j=0;j<m;j++)
            A[i][j] = rand()%100;
    
    for(int i=0;i<m;i++)
        for(int j=0;j<p;j++)
            B[i][j] = rand()%100;
    
    for(int i=0;i<n;i++)
        for(int j=0;j<p;j++)
            C[i][j] = 0;
    
    time_t start, end;
    
    time(&start);
    //normal
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<p;j++)
        {
            for(int k=0;k<m;k++)
            {
                C[i][j] += A[i][k]*B[k][j];
            }
        }
    }
    time(&end);
    printf("time taken to multiply in serial (%d x %d) and (%d x %d) = %f sec\n"
            , n, m, m, p, (double)(end-start));
    
    time_t start2, end2;
    time(&start2);
    //parallel
    #pragma omp parallel for
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<p;j++)
        {
            for(int k=0;k<m;k++)
            {
                C[i][j] += A[i][k]*B[k][j];
            }
        }
    }
    time(&end2);
    printf("time taken to multiply in parallel (%d x %d) and (%d x %d) = %f sec\n"
            , n, m, m, p, (double)(end2-start2));
    return 0;
}