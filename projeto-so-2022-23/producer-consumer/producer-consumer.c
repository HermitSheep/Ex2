#include "utility_funcs.c"
#include "producer-consumer.h"


/*OBRIGATÓRIO: usar     while  (condição)  wait*/				/*usando as condições de variável com a ajuda das funções:
																			wait;signal;broadcast*/

/*
	wait():	Associação entre a secção crítica representada pelo lock e a variavel de condição
	signal(): só é útil usar se houver uma fila da variável de condição  de tarefas, desbloqueando UMA ÚNICA tarefa. 
					nota: não tem memória.
	broadcast(): mesmo que o signal, com a única difernça é que desbloqueia TODAS as tarefas	
 */



int pcq_create(pc_queue_t *queue, size_t capacity) {
	queue->pcq_capacity = capacity;
	queue->pcq_buffer = (void**) malloc (queue->pcq_capacity * sizeof(void));
	queue->pcq_head = 0;
	queue->pcp_last = -1 ;
	queue->pcp_nItems = 0;
}

int pcq_destroy(pc_queue_t *queue) {	/*usa sempre o primeiro item da fila*/
	

}

int pcq_enqueue(pc_queue_t *queue, void *elem) {
	if (queue->pcp_nItems == queue->pcq_capacity){
		sleep(queue);
	}
	if (queue->pcp_last == queue->pcq_capacity-1){
		queue->pcp_last =-1;
	}
	queue->pcp_last++;
	queue->pcq_buffer[queue->pcp_last] = elem;
	queue->pcp_nItems++;

}

void *pcq_dequeue(pc_queue_t *queue) {
	if (queue->pcp_nItems == 0){
		sleep(queue);
	}
	else{
		int value = queue->data_base_pct[queue->pcq_head++];	//pega o valor e incrementa o primeiro
		if(queue->pcq_head == queue->pcq_capacity){
		queue->pcq_head =0;
		}
	}
	queue->pcp_nItems--;

}