#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define CNT 1000000

int* M[4];

void *toGrey(void *thread_id) {
    int* id = (void*) thread_id;

    float r = M[0][*id]*0.3 + M[1][*id]*0.59 + M[2][*id]*0.11; //C =  R*0.30 + G*0.59 + B*0.11
    M[3][*id] = (int) r;

    // printf("%d = %d*0.3 + %d*0.59 + %d*0.11\n", M[3][*id], M[0][*id], M[1][*id], M[2][*id]);
  
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
    
    
    threads = malloc(x*y * sizeof(pthread_t));
    for(i = 0; i<4; i++) 
        M[i] = malloc(x*y * sizeof(int));

    // printf("Digite a matriz de pixels: \n");
    for(i = 0; i< x*y; i++) 
        fscanf(in, "%d %d %d", &M[0][i], &M[1][i], &M[2][i]);

    for(i = 0; i < x*y; i++) {
        int* id = malloc(sizeof(int));
        *id = i;

        if(pthread_create(&threads[i], NULL, toGrey, (void*)id)){
            printf("\n Falha na criação da thread %d \n", i);
            _exit(3);
        }
    }


    for(i = 0; i < x*y; i++){
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