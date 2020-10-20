#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define C_QTD 15
#define P_QTD 1
#define CNT 30
int produced=0, consumed=0;

pthread_mutex_t qMutex, cntProdMutex, cntConsMutex;
pthread_cond_t notFull, notEmpty;

typedef struct elem{
   int value;
   struct elem *prox;
}Elem;
 
typedef struct blockingQueue{
   unsigned int sizeBuffer, statusBuffer;
   Elem *head,*last;
}BlockingQueue;

BlockingQueue* newBlockingQueue(unsigned int SizeBuffer){
    BlockingQueue* Queue = malloc(sizeof(BlockingQueue));
    Queue->sizeBuffer = SizeBuffer;
    Queue->statusBuffer = 0;
    Queue->head = NULL;
    Queue->last = NULL;

    return Queue;
}

void printBlockingQueue(BlockingQueue* Q){
    Elem* temp;

    printf("Fila: (");

    temp = Q->last;
    while(temp!=NULL){
        printf("%d -> ", temp->value);
        temp = temp->prox;
    }
    printf("NULL)\n");
}

void putBlockingQueue(BlockingQueue* Q, int newValue){
    pthread_mutex_lock(&qMutex);

    while(Q->statusBuffer == Q->sizeBuffer){
        printf("\nTa cheio, vou dormir\n");
        pthread_cond_wait(&notFull, &qMutex);
        // printf("\nAcordei\n");
    }

    Elem* newElem = malloc(sizeof(Elem));
    
    newElem->value = newValue;
    newElem->prox = Q->last;
    Q->last = newElem;

    if(!Q->statusBuffer)
        Q->head = Q->last;

    // printf("\nP: %d\nQ_Elementos: %d -> %d\n", newValue, Q->statusBuffer, Q->statusBuffer+1);
    Q->statusBuffer++;

    // printBlockingQueue(Q);

    if(Q->statusBuffer == 1){
        // printf("N ta mais vazio!\n");
        pthread_cond_broadcast(&notEmpty);
    }

    pthread_mutex_unlock(&qMutex);
}

int takeBlockingQueue(BlockingQueue* Q){
    pthread_mutex_lock(&qMutex);

    while(Q->statusBuffer == 0){
        printf("\nTa vazio, vou dormir\n");
        pthread_cond_wait(&notEmpty, &qMutex);
        // printf("\nAcordei\n");
    }

    Elem* temp;
    int ret = Q->head->value;

    temp = Q->last;

    if(Q->statusBuffer == 1){
        Q->last = NULL;
        temp = NULL;
    }
    else{
        while(temp->prox->prox != NULL)
            temp = temp->prox;

        temp->prox = NULL;
    }
    // printf("\nCons: %d\nQ_Elementos: %d -> %d\n", ret, Q->statusBuffer, Q->statusBuffer-1);

    free(Q->head);
    Q->head = temp;
    Q->statusBuffer--;

    // printBlockingQueue(Q);

    if(Q->statusBuffer == Q->sizeBuffer-1){
        // printf("N ta mais cheio!\n");
        pthread_cond_broadcast(&notFull);
    }

    pthread_mutex_unlock(&qMutex);
    return ret;
}

void *prod(void *args) {
    BlockingQueue* Q = ((BlockingQueue**) args)[0];
    int id = *(int*)(((BlockingQueue**) args)+1);
    
    for(int i = id; i<CNT; i+=P_QTD){

        putBlockingQueue(Q, i);
    }
    // printf("\nCabosse a produção de P%d\n", id);

    pthread_exit(args);
} 

void *cons(void *args) {
    BlockingQueue* Q = ((BlockingQueue**) args)[0];
    int id = *(int*)(((BlockingQueue**) args)+1);
    
    for(int i = id; i<CNT; i+=C_QTD){

        takeBlockingQueue(Q);
    }
    // printf("\nCabosse o consumo de C%d\n", id);
  
    pthread_exit(args);
} 


int main(int argc, char *argv[]) {

    pthread_t cThr[C_QTD], pThr[P_QTD];
    int i, j, x, y, maxcollor;

    if (pthread_mutex_init(&qMutex, NULL) != 0) {
      printf("\n Falha na criação do mutex de fila \n");
      _exit(3);
    }

    if (pthread_mutex_init(&cntProdMutex, NULL) != 0) {
      printf("\n Falha na criação do mutex de contador de produção \n");
      _exit(3);
    }

    if (pthread_mutex_init(&cntConsMutex, NULL) != 0) {
      printf("\n Falha na criação do mutex de contador de consumo \n");
      _exit(3);
    }

    if (pthread_cond_init(&notFull, NULL) != 0) {
      printf("\n Falha na criação da condição de fila nao cheia \n");
      _exit(3);
    }

    if (pthread_cond_init(&notEmpty, NULL) != 0) {
      printf("\n Falha na criação do condição de fila nao vazia \n");
      _exit(3);
    }

    BlockingQueue* Q = newBlockingQueue(20);
    
    for(i = 0; i < P_QTD; i++) {
        void*args = malloc(sizeof(BlockingQueue*) + sizeof(int));
        ((BlockingQueue**) args)[0] = Q;
        *(int*)(((BlockingQueue**) args)+1) = i;
        if(pthread_create(&pThr[i], NULL, prod, args)){
            printf("\n Falha na criação da thread produtora %d \n", i);
            _exit(3);
        }
    }
    for(i = 0; i < C_QTD; i++) {
        void*args = malloc(sizeof(BlockingQueue*) + sizeof(int));
        ((BlockingQueue**) args)[0] = Q;
        *(int*)(((BlockingQueue**) args)+1) = i;

        if(pthread_create(&cThr[i], NULL, cons, args)){
            printf("\n Falha na criação da thread consumidora %d \n", i);
            _exit(3);
        }
    }

    for(i = 0; i < P_QTD; i++){
        void* args;
        pthread_join(pThr[i], &args);

        // printf("Thread de prod %d acabou\n", *(int*)(((BlockingQueue**) args)+1));
        free(args);
    }
    for(i = 0; i < C_QTD; i++){
        void* args;
        pthread_join(cThr[i], &args);

        // printf("Thread de cons %d acabou\n", *(int*)(((BlockingQueue**) args)+1));
        free(args);
    }

    free(Q);

    pthread_exit(NULL);
}