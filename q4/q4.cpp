#include <iostream>
#include <queue>
#include <pthread.h>

#define N 3
#define TAMANHO_BUFFER 10
#define NUMERO_REQUISICOES 30

using namespace std;

int numThreads = 0;

//Mutex para uso geral
pthread_mutex_t mutexGeral = PTHREAD_MUTEX_INITIALIZER;		
//Mutex para a criacao de threads
pthread_mutex_t mutexThread = PTHREAD_MUTEX_INITIALIZER;    
//condicional para criacao de threads
pthread_cond_t threads = PTHREAD_COND_INITIALIZER;			
//condicional para indicar o buffer vazio
pthread_cond_t isEmpty = PTHREAD_COND_INITIALIZER;	
//condicional para indicar o buffer cheio
pthread_cond_t isFull = PTHREAD_COND_INITIALIZER;
//serve para a funcao pegarResultado saber se ja pode ou nao acessar o buffer de respostas
pthread_mutex_t bufferMutex[NUMERO_REQUISICOES] = { PTHREAD_MUTEX_INITIALIZER };
//serve como condicao para pegar resultado
pthread_cond_t bufferCond[NUMERO_REQUISICOES] = { PTHREAD_COND_INITIALIZER };	  
//um mutex que serve apenas para os prints nao se sobreporem
pthread_mutex_t mutexPrint = PTHREAD_MUTEX_INITIALIZER;     

long long ID = 0;	//ID para cada execucao de funcao
int contador=0;  	//contador global para usar nos argumentos das funções
int threadDisponivel[N]={1};	//determina quais threads estao disponiveis para serem criadas(inicialmente todas)

//classe para criar um Buffer de Saida:
class bufferResp{
	public:
		bool retorno;    //retorno da função
		int disponivel; //informa se a função está disponível para ser consultada
		
		bufferResp(){
			this->disponivel=0;		//inicializa a disponibildiade com 0
		}
};
bufferResp bufferResposta[NUMERO_REQUISICOES]; //Cria um buffer de saida

//cria uma estrutura que recebe uma funcao e um argumento, (o valor A sendo o valor numerico do argumento
// e o arg um ponteiro para ele)
class func{
    public:
    bool (*funcao)(void*);  	//ponteiro apontando para a função à ser executada
    void *arg;
    int a;
    int id;
};

queue<func> buffer;

bool impar(void *num){
    int arg = *((int *)num);
    int counter;
    bool isImpar = true;
    
    if(arg % 2 == 0){
        isImpar = false;
    }
    
    if (isImpar){
        cout << arg << " é um número impar!" << endl;
        return true;
    }
    else{
        cout << arg << " não é um número impar!" << endl;
        return false;
    }
}

int geradorId(int id){      //funcao para gerar IDS
	return (id+1);
}

int agendarExecucao(void *estrutura){ //Funcao que gera ids para colocar no buffer (funciona como um produtor)
 
    func funcaoAtual=*((func*) estrutura);	  //recebe uma estrutura q contem a funcao e o argumento da funcao
    funcaoAtual.id = ID;					  //da um id para essa estrutura;
    pthread_mutex_lock(&mutexGeral);
    
    //condição para não adicionar novos elementos até o buffer não estar cheio
    while(buffer.size() == TAMANHO_BUFFER){
        pthread_cond_wait(&isEmpty, &mutexGeral);
    }
    
    buffer.push(funcaoAtual);   //se nao tiver cheio, coloca a estrutura no buffer
    if(buffer.size() == 1){
        pthread_cond_broadcast(&isFull); //se tiver pelo menos 1 elemento no buffer, acorta todos os consumidores
    }
    pthread_mutex_unlock(&mutexGeral); 

    ID=geradorId(ID);  //Incrementa o contador global de ID
    //return ID;
}

void *ExecutaThread(void *arg){  //funcao que vai receber a estrutura do buffer e vai executar a funcao e salvar o retorno no buffer de resultado
    func b = *((func *)arg);
        
    pthread_mutex_lock(&bufferMutex[b.id]); //serve para travar o acesso a determinada posicao do buffer de resultado

    bool resposta = b.funcao(&b.a);    							//executa a funcao com seu argumento
    pthread_mutex_lock(&mutexThread);  			//trava mutex de criacao de threads 
    numThreads--;								//diminui o numero de threads sendo usados no momento
    
    //destrava o criador de threads;(serve como consumidor de threads)
    if(numThreads== N-1){ 
        pthread_cond_signal(&threads); 
    }

    pthread_mutex_unlock(&mutexThread);			//destrava o mutex de criacao de threads
    bufferResposta[b.id].retorno=resposta;			//enche o buffer de resposta com a respectiva resposta
    bufferResposta[b.id].disponivel=1;          //diz q a resposta esta disponivel
    pthread_cond_signal(&bufferCond[b.id]);     //Acorda a funcao de pegarResultado caso ela tenha tentando pegar um resultado que ainda nao esta pronto

    pthread_mutex_unlock(&bufferMutex[b.id]);
}

void despachante(void *argument){  	   	         //essa funcao serve como consumidor do buffer e como produtor de threads
    pthread_t *listaThreads;
    listaThreads = new pthread_t[N];
    int i=0;
    func *argumento=(func *) malloc(NUMERO_REQUISICOES*sizeof(func));
    pthread_mutex_lock(&mutexGeral);
	
    while(buffer.size() == 0){   						//consumidor do buffer de requisicoes
        pthread_cond_wait(&isFull, &mutexGeral); 			//dorme caso buffer esteja vazio
    }


    pthread_mutex_lock(&mutexThread);

    while(numThreads==N){ 	
        pthread_cond_wait(&threads, &mutexThread); //dorme caso nao possa criar threads
    }   

    
    do{
        if(threadDisponivel[i]==1){				//procura qual thread ta disponivel para ser criada
            argumento[contador]=buffer.front();	//pega o primeiro item da lista buffer de requisicoes
            buffer.pop();						//diminui o size do buffer em um;
            pthread_create(&listaThreads[i], NULL, ExecutaThread, &argumento[contador]);//cria a thread que executa a funcao recebida
            threadDisponivel[i]==0; 			//diz q tal thread nao esta mais disponivel para ser criada
            numThreads++;						//aumenta o numero de threads
            contador++;							//passa para proximo argumento (para nao embaralhar os argumentos)
            pthread_mutex_unlock(&mutexThread); 
        }
        i++;
    } while(threadDisponivel[i-1]==0); //serve para procurar uma thread q nao esta sendo utilizada no momento 


    if(buffer.size() == TAMANHO_BUFFER-1){
        pthread_cond_signal(&isEmpty);
    } //caso tenha espaco para produtor colocar no buffer, acorda o produtor


    pthread_mutex_unlock(&mutexGeral);
   
}

void *despachador(void *argument){ 					//serve apenas para determinar quantas requisicoes o despachador vai tirar do buffer
    for(int i=0; i < NUMERO_REQUISICOES; i++){
        despachante(NULL);
    }
}

int pegarResultadoExecucao(int id){						//essa funcao serve para pegar as requisicoes do buffer de resultados(de saida) e ver os valores de retorno
	pthread_mutex_lock(&bufferMutex[id]);				//ve se esta disponivel para ser acessado
	
    while(bufferResposta[id].disponivel==0){			//se nao estiver diponivel, dorme
		pthread_cond_wait(&bufferCond[id], &bufferMutex[id]);
    }

    pthread_mutex_lock(&mutexPrint);
    if(bufferResposta[id].retorno){ 
        cout << "Recebi: verdadeiro, da execucao do id: "<<id<<endl;  //mostra o valor que recebeu
    } else{
        cout << "Recebi: falso, da execucao do id: "<<id<<endl;  //mostra o valor que recebeu
    }
    bufferResposta[id].disponivel==0;                                                   //abre espaco no buffer de saida
    pthread_mutex_unlock(&mutexPrint);
    pthread_mutex_unlock(&bufferMutex[id]);
}


int main(void) {
    pthread_t *threads;
    threads = new pthread_t[N];
    long *taskids = new long[N];
    func *minhaFuncao=(func *)malloc(NUMERO_REQUISICOES*sizeof(func));
    int rc;
    pthread_create(&threads[0], NULL, despachador, NULL);//cria um despachador que vai rodar simultaneamente no codigo

   for(int t=0; t < NUMERO_REQUISICOES; t++){
        minhaFuncao[t].funcao = *impar;			//recebe a funcao generica(funexec)
        minhaFuncao[t].a = t; 
        minhaFuncao[t].arg = &minhaFuncao[t].a;
        agendarExecucao(&minhaFuncao[t]);		//chama a funcao para acolocar as requisicoes no buffer
	}
  
    for(int t =0; t < NUMERO_REQUISICOES; t++){
    	pegarResultadoExecucao(rand()%NUMERO_REQUISICOES);	//serve para um acesso aleatorio de Ids(pode ser sequencial tambem, mas resolvemos fazer dess ejeito. Depende da aplicacao);
    	//pegarResultadoExecucao(t);
	} 
   
    pthread_join(threads[0],NULL); 

    return 0;
}
