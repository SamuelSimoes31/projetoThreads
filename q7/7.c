#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define Q_THREADS 16

int* M[4];
int npixels, nthreads = Q_THREADS;

void *toGrey(void *thread_id) {
    int id = *((int*) thread_id);
    int i;
    float r;

    for(i = id; i<npixels; i+= nthreads){

        r = M[0][i]*0.3 + M[1][i]*0.59 + M[2][i]*0.11; //C =  R*0.30 + G*0.59 + B*0.11
        M[3][i] = (int) r;

        // printf("T%d->%d : %d = %d*0.3 + %d*0.59 + %d*0.11\n", id, i, M[3][i], M[0][i], M[1][i], M[2][i]);
    }
  
    pthread_exit(thread_id);
} 


int main(int argc, char *argv[]) {

    FILE *in, *out;
    pthread_t* threads;
    int i, j, x, y, maxcollor;

    in = fopen("7in.ppm", "r");
    if(in == NULL){
        printf("\n Falha na abertura do arquivo de entrada \n");
        _exit(3);
    }

    out = fopen("7out.ppm", "w+");
    if(out == NULL){
        printf("\n Falha na abertura do arquivo de saida \n");
        _exit(3);
    }


    // printf("Digite as dimensões x, y e o valor máximo: \n");
    // char *palette = malloc(10*sizeof(char));
    // fscanf(in, "%s", palette);
    fscanf(in, "%d %d %d", &x, &y, &maxcollor);
    
    npixels = x*y;
    if(npixels < nthreads)
        nthreads = npixels;
    
    threads = malloc(nthreads * sizeof(pthread_t));
    for(i = 0; i<4; i++) 
        M[i] = malloc(npixels * sizeof(int));

    // printf("Digite a matriz de pixels: \n");
    for(i = 0; i < npixels; i++) 
        fscanf(in, "%d %d %d", &M[0][i], &M[1][i], &M[2][i]);

    for(i = 0; i < nthreads; i++) {
        int* id = malloc(sizeof(int));
        *id = i;

        if(pthread_create(&threads[i], NULL, toGrey, (void*)id)){
            printf("\n Falha na criação da thread %d \n", i);
            _exit(3);
        }
    }

    for(i = 0; i < nthreads; i++){
        void* id;

        pthread_join(threads[i], &id);
        // printf("%d\n",*((int*) id));

        free((int*)id);
    }
    
    // printf("\n");
    // fprintf(out, "%s\n%d %d\n%d\n", palette, x, y, maxcollor);
    fprintf(out, "%d %d\n%d\n", x, y, maxcollor);

    for(i = 0; i < y; i++){
      
        for(j = 0; j<x; j++){
            fprintf(out,"%d %d %d", M[3][i*x + j], M[3][i*x + j], M[3][i*x + j]);
            if(i != y-1 || j != x-1) fprintf(out, "\n");

            // printf("%d ", i*x + j);
        }
            
        // printf("\n");
        // if(i < y-1) fprintf(out, "\n");
    }

    free(threads);
    for(i = 0; i<4; i++) 
        free(M[i]);

    fclose(in);
    fclose(out);

    pthread_exit(NULL);
}