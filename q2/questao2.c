#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

int n_lines, n_files, n_threads, open_file=0;
pthread_mutex_t *mutex_lines = NULL;
pthread_mutex_t mutex_open_file = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_print = PTHREAD_MUTEX_INITIALIZER;

void print_line(int line, char str[26], int len){
    int i=len;
    pthread_mutex_lock(&mutex_print);
    printf("\e[%d;1H",line);
    printf("%s",str);
    while (i++ < 20){
        putchar(' ');
    }
    printf("%s",str+len+1);
    fflush(stdout);
    pthread_mutex_unlock(&mutex_print);
}

void *thread_func(){
    FILE *file = NULL;
    char file_name[7]={0};

    int line,len;
    char string[26];

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
        while(!feof(file)){
            fscanf(file,"%d",&line);
            pthread_mutex_lock(&mutex_lines[line-1]);

            fscanf(file," %[^\n]",string);
            len = strlen(string)-6;
            string[len] = 0;
            print_line(line,string,len);
            sleep(2);

            pthread_mutex_unlock(&mutex_lines[line-1]);
        }
        

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
    printf("\e[H"); //seta cursor pra posição inicial
    printf("\e[J"); //limpa tudo a partir do cursor
    printf("Número de arquivos:");
    scanf("%d",&n_files);
    printf("Número de linhas:");
    scanf("%d",&n_lines);
    printf("Número de número de threads:");
    scanf("%d",&n_threads);
    // fflush(stdin);
    printf("\e[H"); //seta cursor pra posição inicial
    printf("\e[J"); //limpa tudo a partir do cursor
    printf("\e[?25l"); //esconde o cursor

    //ALOCAÇÕES
    mutex_lines = (pthread_mutex_t *)malloc(n_lines*sizeof(pthread_mutex_t));
    for(i=0;i<n_lines;i++) pthread_mutex_init(&mutex_lines[i],NULL);

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

    printf("\e[H"); //seta cursor pra posição inicial
    printf("\e[J"); //limpa tudo a partir do cursor
    printf("\e[?25h"); //exibe o cursor

    //FREE
    free(mutex_lines);
    free(threads);
    return 0;
}