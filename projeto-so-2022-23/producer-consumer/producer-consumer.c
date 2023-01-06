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

}

int pcq_destroy(pc_queue_t *queue) {

}

int pcq_enqueue(pc_queue_t *queue, void *elem) {

}

void *pcq_dequeue(pc_queue_t *queue) {

}