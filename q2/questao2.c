#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

int n_files, open_file=0;
pthread_mutex_t *mutex_lines = NULL; //array de mutexes, uma para cada linha
pthread_mutex_t mutex_open_file = PTHREAD_MUTEX_INITIALIZER; //mutex para anecessar variável "open_file" que diz qual o próximo arquivo a ser aberto
pthread_mutex_t mutex_print = PTHREAD_MUTEX_INITIALIZER; //mutex para imprimir uma linha, pois o uso do cursosr é uma condição de disputa

void set_line_color(int line){
    if(line != 6){
        printf("\e[38;2;0;0;0m"); //seta cor do texto para preto
        switch(line){
            //setar cor do fundo
            case 1: {printf("\e[48;2;255;0;0m");break;}     //texto vermelho
            case 2: {printf("\e[48;2;255;255;0m");break;}   //texto amarelo
            case 3: {printf("\e[48;2;0;0;255m");break;}     //texto azul
            case 4: {printf("\e[48;2;255;0;255m");break;}    //texto magenta
            case 5: {printf("\e[48;2;0;255;0m");break;}      //texto verde
            case 7: {printf("\e[48;2;0;255;255m");break;}    //texto ciano
        }
    } else {
        printf("\e[38;2;255;255;255m"); //seta cor do texto para branco
        printf("\e[48;2;0;0;0m"); //texto preto
    }
}

void print_line(int line, char *str, int len){
    int i=len;
    pthread_mutex_lock(&mutex_print);
    printf("\e[%dH",line); //seta cursor pra linha "line" e coluna 1 (\e[%d;1H == \e[%dH)
    set_line_color(line);
    printf("%s",str); //imprime código e cidade
    while (i++ < 20){ //espaços até a posição 20
        putchar(' ');
    }
    printf("%s",str+len+1); //imprime hora
    fflush(stdout); //dá um flush na saída pois estava havendo alguns bugs
    pthread_mutex_unlock(&mutex_print);
}

void clean_screen(int show_cursor){
    printf("\e[H"); //seta cursor pra posição inicial \e[H == \e[1;1H
    printf("\e[J"); //limpa tudo a partir do cursor
    printf("\e[?25%c",show_cursor?'h':'l'); //(?25h exibe, ?25l esconde) o cursor
}

void *thread_func(){
    FILE *file = NULL;
    char file_path[13]={0};

    int line,len;
    char string[26];

    pthread_mutex_lock(&mutex_open_file);
    while(open_file < n_files){
        sprintf(file_path,"files/%d.txt",open_file);
        file = fopen(file_path,"r");
        if(file==NULL){
            printf("Erro ao abrir arquivo %s\n",file_path);
            pthread_mutex_unlock(&mutex_open_file);
            pthread_exit(NULL);
        }
        open_file++;
        pthread_mutex_unlock(&mutex_open_file);
        
        /*MODIFICA LINHAS*/
        while(!feof(file)){
            fscanf(file," %d",&line);
            pthread_mutex_lock(&mutex_lines[line-1]);

            fscanf(file," %[^\n]",string);
            len = strlen(string)-6;
            string[len] = 0; //setar o fim da string na posição antes do horário
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
    int i,rc, n_lines, n_threads;
    pthread_t *threads = NULL;
    clean_screen(1);
    do{
        printf("Número de arquivos a serem lidos (no mínimo 1):");
        scanf("%d",&n_files);
    } while(n_files < 1);
    do{
        printf("Número de linhas(1 à 7):");
        scanf("%d",&n_lines);
    } while(n_lines<1 || n_lines>7);
    
    do{
        printf("Número de threads que irão ler os arquivos (1 à número de arquivos):");
        scanf("%d",&n_threads);
    } while(n_threads <1 && n_threads > n_files);
    clean_screen(0);

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

    printf("\e[0m"); //reseta cores para padrão
    clean_screen(1);

    //FREE
    free(mutex_lines);
    free(threads);
    return 0;
}