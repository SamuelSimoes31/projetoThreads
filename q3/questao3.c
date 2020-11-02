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

int qtd = 0;
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

void *contar_substring(StrPointers *ptr){
    int i,j,ret=0,len=strlen(ptr->agulha);
    for(i=ptr->base; i<ptr->limite; i++){
        if(ptr->palheiro[i] == ptr->agulha[0] && i+len-1 < ptr->limite){
            for(j=1;j<len;j++){
                if(ptr->palheiro[j] != ptr->agulha[j]) break;
                else if(j == len-1) ret++;
            }
        }
    }
    ptr->base = ret;
    pthread_mutex_lock(&mymutex);
    qtd +=ret;
    pthread_mutex_unlock(&mymutex);
    pthread_exit((void *)&ptr->base);
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
        printf("res: %d\n",*res);
        ret += *res;
    }

    free(strp);
    free(threads);
    return ret;
}

int main(void) {
    printf("%d",quantidade_substring("abcdab","ab"));
    printf("qtd=%d",qtd);
    return 0;
}