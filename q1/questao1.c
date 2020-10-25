#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5
#define LIM_CONTADOR 1000000

long contador = 0; 
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

void *threadFunc(void *threadid){
    int i;
    for(i=0;i<LIM_CONTADOR;i++){
        pthread_mutex_lock(&mymutex);
        if(contador < LIM_CONTADOR){
            contador++;
            printf("t%d -> %ld\n",*(int *)threadid, contador);
            if(contador == LIM_CONTADOR) {
                printf("t%d: ALCANCEI %d. GANHEI\n",*(int *)threadid,LIM_CONTADOR);
                break;
            }
            pthread_mutex_unlock(&mymutex);
        }
        else{
            printf("t%d: perdi\n",*(int *)threadid);
            break;
        }
    }
    pthread_mutex_unlock(&mymutex);
    pthread_exit(NULL);
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
    }
    for(t=0; t<NUM_THREADS; t++) pthread_join(threads[t], NULL);
    for(t=0; t<NUM_THREADS; t++) free(taskids[t]); 

    return 0;
}