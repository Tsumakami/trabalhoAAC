//Trabalho de Arquitetura Avançada de computadores
//Trabalho realizado por:
//	João Paulo Malvesti,
//	Guilherme Portigliotti
//	Joni Milczarek

//Entradas
/* source,target,mensagem
*/

// Um programa.c que vai ter uma main ()
// 	Solicitar para o usuário quantidade de nodos e chaveamento
// 	Conforme a qtd de nodos de chaveamento, então, vcs vão criar as threads para cada uma
// 	Na main terão um array com mensagens a serem trafegadas da rede
// 		Mensangens:
// 		Enviar "b = 100 do nodo 1 para o nodo 2"
// 		Enviar "x++ do nodo 2 para o nodo 4"
// 		...


// 		Ex: 4 nodos de chaveamento
// 	1  2  3  4
 
// 	[nodo 1] enviando mensagem "b = 100" para nodo 2
// 	[nodo 2] recebido mensagen "b = 100" do nodo 1 [source=1]
// 	[nodo 2] enviando mensagem "x++" para nodo 4
// 	[nodo 3] recebida mensagem "x++" do nodo 2
// 	[nodo 3] enviando mensagem "x++" para o nodo 4
// 	[nodo 4] recebida mensagem "x++" do nodo 3 [source=2]


//Trafegar as mensagens pelo menor caminho
//Por nodo printar, quantas mensagens criadas, quantas recebidas e quantas trafegadas
//Msg Criadas: i
//Msg Destino: i
//Msg Trafegadas: i

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

//Variaveis de controle
#define LIMIT_MAX_THREAD 100
int debug = 0;

int qt_nodos = 1;
int countMsgCriadas[1000] = {0};
int countMsgDestino[1000] = {0};
int countMsgtrafeg[1000] = {0};

pthread_t threads[LIMIT_MAX_THREAD];

struct InputForProcess{
    int source;
    int target;
    char message[100];
    int direction;
    long position;
};

//Estrutura de memória compartilhada do InputForProcess
struct InputForProcess memoryShared[LIMIT_MAX_THREAD];

void increseMsgCount(int type, int nodo) {
    if (type == 1) {
        countMsgCriadas[nodo]++;
    } else if (type == 2) {
        countMsgDestino[nodo]++;
    } else if (type == 3) {
        countMsgtrafeg[nodo]++;
    }
    
    if(debug) printf("[DEBUG] Nodo: %d, Criadas: %d, Destino: %d, Trafegada: %d\n", nodo, countMsgCriadas[nodo], countMsgDestino[nodo], countMsgtrafeg[nodo]);
}

int direction(struct InputForProcess input){
    int result = 0;
    int forRight = input.target - input.source;
    int forLeft = input.source - input.target;

    if(forLeft < 0){
    	forLeft = qt_nodos + forLeft;
    }else {
    	forRight = qt_nodos + forRight;
    }

    if(forRight <= forLeft){
    	result = 1;
        if(debug) printf("[DEBUG] direction - result: %d = right\n", result);
    }else{
	    result = -1;
        if(debug) printf("[DEBUG] direction - result: %d = left\n", result);
    }

    return result;
}

int nextPosition(int position, int direction){
    int result = -1;

    if(position + direction == -1){
        result = qt_nodos - 1;
    }else if(position + direction == qt_nodos){
        result = 0;
    }else{
        result = direction + position;
    }

    if(debug) printf("[DEBUG] nextPosition - proximo nodo %d\n", result);
    
    return result;
}

int identifyReceivedNodo(long position, long direction) {
    int result = -1;

    if(position - direction == -1){
        result = qt_nodos - 1;
    }else if(position - direction == qt_nodos){
        result = 0;
    }else{
        result = direction - position;
        if(result < 0){
            result = -result;
        }
    }

    if(debug) printf("[DEBUG] identifyReceivedNodo - recebido do nodo %d\n", result);
    
    return result;
}

void processMessage(long nodo){
    int received = identifyReceivedNodo(memoryShared[nodo].position, memoryShared[nodo].direction);
    long nextPos = nextPosition(memoryShared[nodo].position, memoryShared[nodo].direction);
    
    if(memoryShared[nodo].source == memoryShared[nodo].position){
        printf("[nodo %ld] enviando mensagem %s para nodo %ld [target=%d]\n", memoryShared[nodo].position, memoryShared[nodo].message, nextPos, memoryShared[nodo].target);
        
        memoryShared[nodo].position = nextPos;
        memoryShared[memoryShared[nodo].position] = memoryShared[nodo];

    } else if (memoryShared[nodo].target == memoryShared[nodo].position){
        printf("[nodo %ld] recebido mensagen %s do nodo %d [source=%d]\n", memoryShared[nodo].position, memoryShared[nodo].message, received, memoryShared[nodo].source);
        
        memoryShared[nodo].position = -1;
    }else {
        printf("[nodo %ld] recebida mensagem %s do nodo %d\n", memoryShared[nodo].position, memoryShared[nodo].message, received);
        printf("[nodo %ld] enviando mensagem %s para o nodo %ld\n", memoryShared[nodo].position, memoryShared[nodo].message, nextPos);
        
        increseMsgCount(3, memoryShared[nodo].position);

        memoryShared[nodo].position = nextPos;
        memoryShared[memoryShared[nodo].position] = memoryShared[nodo];
    }

}

//Declaração da função para compilar
void nextThread(long nodo);

void executeTread(long nodo){
    
    processMessage(nodo);
    
    if(memoryShared[nodo].position != -1){
        nextThread(memoryShared[nodo].position);
    }

    pthread_exit(NULL);
}

void nextThread(long nodo){
    int creatStatus = 0;

    creatStatus = pthread_create(&threads[nodo], NULL, (void *)executeTread,(void *) nodo);

    if(creatStatus == 0){
        if(debug) printf("[DEBUG] nextThread - Thread %ld foi criada com sucesso!\n", nodo);
        pthread_join(threads[nodo], NULL);
    }else{
        if(debug) printf("[DEBUG] nextThread - Thread %ld retornou status %d e falhou\n", nodo, creatStatus);
    }    
}

void statistic(){
    printf("\n****** Estatisticas de Trafego ******\n");
    for(int nodo = 0; nodo < qt_nodos; nodo++){
        printf("[nodo %d] Criadas: %d, Destino: %d, Trafegada: %d\n", nodo, countMsgCriadas[nodo], countMsgDestino[nodo], countMsgtrafeg[nodo]);
    }
}

int main(int argc, char *argv[]){
    scanf("%d", &qt_nodos);    
    
    struct InputForProcess input;
    while(scanf("%d,%d,%[^\n]", &input.source, &input.target, input.message) != EOF){
        input.position = input.source;
        input.direction = direction(input);
        memoryShared[input.position] = input;

        if(debug) printf("[DEBUG] Input - Source: %d, Target: %d, Message: %s\n", input.source, input.target, input.message);
        
        increseMsgCount(1, input.source);
        increseMsgCount(2, input.target);

        nextThread(memoryShared[input.position].position);        

    }
    statistic();
}

