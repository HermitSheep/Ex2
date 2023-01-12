#include "../utils/utility_funcs.h"
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
	queue->pcq_tail = (size_t) -1 ;
	queue->pcq_current_size  = 0;
	if (queue->pcq_buffer == NULL){
		return -1;
	}
	queue->pcq_capacity = capacity;
	    
	// Initialize the mutexes and condition variables
    pthread_mutex_init(&queue->pcq_current_size_lock, NULL);
    pthread_mutex_init(&queue->pcq_head_lock, NULL);
    pthread_mutex_init(&queue->pcq_tail_lock, NULL);
    pthread_mutex_init(&queue->pcq_pusher_condvar_lock, NULL);
    pthread_mutex_init(&queue->pcq_popper_condvar_lock, NULL);
    pthread_cond_init(&queue->pcq_pusher_condvar, NULL);
    pthread_cond_init(&queue->pcq_popper_condvar, NULL);

    return 0;


}

int pcq_destroy(pc_queue_t *queue) {	
    // Free the queue buffer
    free(queue->pcq_buffer);

    // Destroy the mutexes and condition variables
    pthread_mutex_destroy(&queue->pcq_current_size_lock);
    pthread_mutex_destroy(&queue->pcq_head_lock);
    pthread_mutex_destroy(&queue->pcq_tail_lock);
    pthread_mutex_destroy(&queue->pcq_pusher_condvar_lock);
    pthread_mutex_destroy(&queue->pcq_popper_condvar_lock);
    pthread_cond_destroy(&queue->pcq_pusher_condvar);
    pthread_cond_destroy(&queue->pcq_popper_condvar);

	return 0;
}

int pcq_enqueue(pc_queue_t *queue, void *elem) {
	pthread_mutex_lock(&queue->pcq_current_size_lock);

	while (queue->pcq_current_size == queue->pcq_capacity) {
        pthread_cond_wait(&queue->pcq_pusher_condvar, &queue->pcq_pusher_condvar_lock);
    }
	pthread_mutex_lock(&queue->pcq_tail_lock);
	if (queue->pcq_tail == queue->pcq_capacity-1){
		queue->pcq_tail = (size_t) -1;
	}
	queue->pcq_tail++;								//*ultimo elemento a ser adicionado
	queue->pcq_buffer[queue->pcq_tail] = elem;
	queue->pcq_current_size ++;
	pthread_mutex_unlock(&queue->pcq_current_size_lock);
	pthread_mutex_unlock(&queue->pcq_tail_lock);

}

void *pcq_dequeue(pc_queue_t *queue) {
	pthread_mutex_lock(&queue->pcq_current_size_lock);
	if (queue->pcq_current_size  == 0){							
		pthread_cond_wait(&queue->pcq_popper_condvar, &queue->pcq_popper_condvar_lock);
	}
	else{
		pthread_mutex_lock(&queue->pcq_head_lock);
		int value = queue->pcq_buffer[queue->pcq_head++];	//*primeiro a ser retirado
		if(queue->pcq_head == queue->pcq_capacity){
		queue->pcq_head =0;
		}
	}
	queue->pcq_current_size --;
	pthread_mutex_unlock(&queue->pcq_popper_condvar_lock);
	pthread_mutex_unlock(&queue->pcq_head_lock);


}