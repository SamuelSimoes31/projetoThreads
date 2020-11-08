#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define Q_THREADS 16
#define M_SIZE 4
#define MAX_ITERATIONS 100

int nthreads = Q_THREADS, m_size = M_SIZE, max_iterations = MAX_ITERATIONS;
double MA[M_SIZE][M_SIZE] = {
    {4, 1, 1, 1},
    {1, 4, 1, 1},
    {1, 1, 4, 1},
    {1, 1, 1, 4}
};
double MB[M_SIZE] = {6, 
                    -1,
                    3,
                    -2};
double MX[M_SIZE] = {1, 
                    1,
                    1,
                    1};

pthread_barrier_t iterationBarrier;

void *jacobiThread(void *args) {
    int id = *(int*)args;
    int i, j, k;
    double sum, nxtMX[M_SIZE];

    for(k = 0; k<max_iterations; k++){

        for(i = id; i<m_size; i+= nthreads){
            sum = 0;
            
            for(j = 0; j<m_size; j++){
                if(j!=i){
                    sum+=MA[i][j]*MX[j];
                }
            }
            nxtMX[i] = (MB[i] - sum)/MA[i][i]; 

        }
        pthread_barrier_wait(&iterationBarrier);
        
        for(i = id; i<m_size; i+= nthreads)
            MX[i] = nxtMX[i];

        pthread_barrier_wait(&iterationBarrier);
    }
       
    free((int*)args);

    pthread_barrier_wait(&iterationBarrier);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    
    pthread_t* threads;
    int i;

    pthread_barrier_init(&iterationBarrier, NULL, nthreads);

    threads = malloc(sizeof(pthread_t)*nthreads);
    if(threads == NULL){
        printf("\n Falha na criação do vetor de threads\n");
        _exit(3);
    }

    for(i = 0; i < nthreads; i++) {
        int* id = malloc(sizeof(int));
        *id = i;

        if(pthread_create(&threads[i], NULL, jacobiThread, (void*)id)){
            printf("\n Falha na criação da thread %d \n", i);
            _exit(3);
        }
    }

    pthread_join(threads[0], NULL);

    free(threads);

    for(i = 0; i<m_size; i++){
        printf("x[%d] = %lf\n", i, MX[i]);
    }

    pthread_exit(NULL);
}
