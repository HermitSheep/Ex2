#ifndef __PRODUCER_CONSUMER_H__
#define __PRODUCER_CONSUMER_H__

#include <pthread.h>

// IMPORTANT: do not change anything in this file
//
// This API will be used separately to test your producer consumer
// implementation

typedef struct {
    void **pcq_buffer;                                              //request
    size_t pcq_capacity;

    pthread_mutex_t pcq_current_size_lock;          //queue lock
    size_t pcq_current_size;

    pthread_mutex_t pcq_head_lock;                       //first element lock
    size_t pcq_head;

    pthread_mutex_t pcq_tail_lock;                          //last element lock
    size_t pcq_tail;

    pthread_mutex_t pcq_pusher_condvar_lock;        //pushing operation
    pthread_cond_t pcq_pusher_condvar;

    pthread_mutex_t pcq_popper_condvar_lock;        //poping operation
    pthread_cond_t pcq_popper_condvar;
} pc_queue_t;


//pthread_cond_t bufferCheio = PTHREAD_COND_INITIALIZER;
//pthread_cond_t msgparaLer = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t semExMut = PTHREAD_MUTEX_INITIALIZER;

// pcq_create: create a queue, with a given (fixed) capacity
//
// Memory: the queue pointer must be previously allocated
// (either on the stack or the heap)
int pcq_create(pc_queue_t *queue, size_t capacity);

// pcq_destroy: releases the internal resources of the queue
//
// Memory: does not free the queue pointer itself
int pcq_destroy(pc_queue_t *queue);

// pcq_enqueue: insert a new element at the front of the queue
//
// If the queue is full, sleep until the queue has space
int pcq_enqueue(pc_queue_t *queue, void *elem);

// pcq_dequeue: remove an element from the back of the queue
//
// If the queue is empty, sleep until the queue has an element  //?shouldn't it do nothing instead?
void *pcq_dequeue(pc_queue_t *queue);

#endif // __PRODUCER_CONSUMER_H__
