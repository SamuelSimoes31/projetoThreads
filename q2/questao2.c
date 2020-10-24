#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int n_lines, n_files, n_threads, open_file=0;
char **lines = NULL;
pthread_mutex_t *mutex_lines = NULL;
pthread_mutex_t mutex_open_file = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_print = PTHREAD_MUTEX_INITIALIZER;

void *thread_func(){
    FILE *file = NULL;
    char file_name[7]={0};

    pthread_mutex_lock(&mutex_open_file);
    while(open_file < n_files){
        sprintf(file_name,"%d.txt",open_file);
        file = fopen(file_name,"r");
        if(file==NULL){
            printf("Erro ao abrir arquivo %s\n",file_name);
            pthread_mutex_unlock(&mutex_open_file);
            pthread_exit(NULL);
        }
        open_file++;
        pthread_mutex_unlock(&mutex_open_file);
        
        /*MODIFICA LINHAS*/
        char teste[50];
        fscanf(file,"%s",teste);
        printf("%s\n",teste);

        fclose(file);
        file = NULL;
        pthread_mutex_lock(&mutex_open_file);
    }
    pthread_mutex_unlock(&mutex_open_file);
    pthread_exit(NULL);
}

int main(void) {
    int i,rc;
    pthread_t *threads = NULL;
    printf("\e[2J");    //limpa tela
    printf("\e[1;1H");  //seta cursor pra posição 1 1
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
    threads = (pthread_t *)malloc(n_threads*sizeof(pthread_t));
    for(i=0;i<n_threads;i++){
        rc = pthread_create(&threads[i],NULL,thread_func,NULL);
        if(rc){
            printf("Erro ao criar thread %d\n",i);
            break;
        }
    }

    if(!rc){
        for(i=0;i<n_threads;i++) pthread_join(threads[i], NULL);
    }

    //FREE
    free(mutex_lines);
    for(i=0;i<n_lines;i++) free(lines[i]);
    free(lines);
    free(threads);
    return 0;
}