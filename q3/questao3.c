#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct{
    int base;
    int limite;
    char *palheiro;
    char *agulha;
}StrPointers;


void *contar_substring(StrPointers *strp){
    int ret=0;
    printf("base:%d limite:%d palheiro:%s agulha:%s\n",strp->base,strp->limite,strp->palheiro,strp->agulha);
    pthread_exit((void *)&ret);
}

int quantidade_substring(char *s1, char *s2){
    int n1,n2,p=1,i,ret=0;
    StrPointers *strp=NULL;
    int *res=NULL;
    pthread_t *threads=NULL;

    n1 = strlen(s1);
    n2 = strlen(s2);
    for(i=2;i*n2<n1;i++){
        if(n1%i==0) p = i;
    }
    printf("p=%d\n",p);
    
    strp = (StrPointers *)malloc(p*sizeof(StrPointers));
    if(strp==NULL){
        printf("Falha ao alocar strings");
        exit(3);
    }

    threads = (pthread_t *)malloc(p*sizeof(pthread_t));
    if(threads==NULL){
        printf("Falha ao alocar threads");
        free(strp);
        exit(3);
    }

    for(i=0;i<p;i++){
        strp[i].palheiro = s1;
        strp[i].agulha = s2;
        strp[i].base = i*(n1/p);
        strp[i].limite = strp[i].base + (n1/p);
        pthread_create(&threads[i],NULL,(void *)contar_substring,&strp[i]);
    }

    for(i=0;i<p;i++){
        pthread_join(threads[i],(void **)&res);
        ret += *res;
    }

    free(strp);
    free(threads);
    return ret;
}

int main(void) {
    printf("%d",quantidade_substring("abcdab","ab"));
    return 0;
}