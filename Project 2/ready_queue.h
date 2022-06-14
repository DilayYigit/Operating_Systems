#ifndef READY_QUEUE_H
#define READY_QUEUE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
pthread_mutex_t lock;

int init_ready_queue(struct ready_queue** rq);
void insert(struct ready_queue *rq, struct thread_node *t);
void remove_node(struct ready_queue *rq, int index);
thread_node* dequeue(ready_queue *rq);
thread_node* find_shortest_burst(ready_queue* rq);
#endif
