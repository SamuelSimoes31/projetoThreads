#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

int main(){
    char cidades[12][15] = {
        "Luxemburgo",
        "Paris",
        "Tokyo",
        "Recife",
        "NovaYork",
        "Seoul",
        "Berlim",
        "Madri",
        "Estocolmo",
        "Madri",
        "Nice",
        "St. Petsbu."
    };
    
    srand(time(NULL));
    int n_files,n_lines,n_cases;
    printf("Número de arquivos: ");
    scanf(" %d",&n_files);
    printf("Número de linhas: ");
    scanf(" %d",&n_lines);
    printf("Número de casos por arquivo: ");
    scanf(" %d",&n_cases);
    mkdir("files",0777);
    for(int i=0;i<n_files;i++){
        char nome[30];
        sprintf(nome,"files/%d.txt",i);
        FILE* file = NULL;
        file = fopen(nome,"w");
        if(file == NULL){
            printf("ERRO AO CRIAR AQRUIVO");
            exit(-1);
        }
        for(int j=0;j<n_cases;j++){
            int line = rand()%n_lines + 1;
            fprintf(file,"%d\n",line);
            fprintf(file,"%d%d%d%c%c%c %s %02d:%02d",
                rand()%10,rand()%10,rand()%10,'A'+rand()%26,'A'+rand()%26,'A'+rand()%26,
                cidades[rand()%12],
                rand()%24,rand()%60);
            if(j<n_cases-1) fprintf(file,"\n");
        }
        fclose(file);
    }
}