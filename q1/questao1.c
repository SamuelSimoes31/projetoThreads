#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5

long contador = 0; 
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

void *threadFunc(void *threadid){
    pthread_mutex_lock(&mymutex);
    if(contador == 1000000) {
        printf("t%d: perdi\n",*(int *)threadid);
        pthread_mutex_unlock(&mymutex);
        pthread_exit(NULL);
    }
    else {
        contador++;
        printf("t%d -> %ld\n",*(int *)threadid, contador);
        if(contador == 1000000) {
            puts("GANHEI PORRA\n");
            pthread_mutex_unlock(&mymutex);
            pthread_exit(NULL);
        }
        else{
            pthread_mutex_unlock(&mymutex);
        }
    }
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    int *taskids[NUM_THREADS];

    int t,i,rc;  
    for(t=0; t<NUM_THREADS; t++){      
        taskids[t] = (int *) malloc(sizeof(int)); *taskids[t] = t;
        printf("No main: criando thread %d\n", t);      
        rc = pthread_create(&threads[t], NULL, threadFunc, (void *) taskids[t]);      
        if (rc){
            for(i=0;i<=t;i++) free(taskids[t]);       
            printf("ERRO; código de retorno é %d\n", rc);         
            exit(-1);      
        }
        pthread_join(threads[t], NULL);
    } 
    for(t=0; t<NUM_THREADS; t++) free(taskids[t]); 

    return 0;
}