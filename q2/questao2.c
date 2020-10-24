#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int n_lines, n_files, n_threads, open_file=0;
char **lines = NULL;
pthread_mutex_t *mutex_lines = NULL;
pthread_mutex_t mutex_open_file = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_print = PTHREAD_MUTEX_INITIALIZER;

int main(void) {
    int i;
    printf("\e[2J");
    printf("\e[1;1H");
    printf("Número de arquivos:");
    scanf("%d",&n_files);
    printf("Número de linhas:");
    scanf("%d",&n_lines);
    printf("Número de número de threads:");
    scanf("%d",&n_threads);

    //ALOCAÇÕES
    mutex_lines = (pthread_mutex_t *)malloc(n_lines*sizeof(pthread_mutex_t));
    lines = (char **)malloc(n_lines*sizeof(char *));
    for(i=0;i<n_lines;i++){
        lines[i] = (char *)calloc(25,sizeof(char));
        mutex_lines[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    }

    //FREE
    free(mutex_lines);
    for(i=0;i<n_lines;i++) free(lines[i]);
    free(lines);
    return 0;
}