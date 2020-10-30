#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void *contar_substring(int *id){
    printf("[%d]\n",*id);
    pthread_exit((void *)id);
}

int quantidade_substring(char *s1, char *s2){
    int n1,n2,p=1,i;
    int *ids=NULL,*res;
    pthread_t * threads=NULL;

    n1 = strlen(s1);
    n2 = strlen(s2);
    for(i=2;i*n2<n1;i++){
        if(n1%i==0) p = i;
    }
    printf("p=%d\n",p);
    ids = (int *)malloc(p*sizeof(int));
    if(ids==NULL){
        printf("Falha ao alocar ids");
        exit(3);
    }
    threads = (pthread_t *)malloc(p*sizeof(pthread_t));
    if(threads==NULL){
        printf("Falha ao alocar threads");
        free(ids);
        exit(3);
    }

    for(i=0;i<p;i++){
        ids[i]=i;
        pthread_create(&threads[i],NULL,(void *)contar_substring,&ids[i]);
    }

    for(i=0;i<p;i++){
        pthread_join(threads[i],(void **)&res);
        printf("%d\n",*res);
    }

    free(ids);
    free(threads);
    return *res;
}

int main(void) {
    quantidade_substring("abcdab","ab");
    return 0;
}