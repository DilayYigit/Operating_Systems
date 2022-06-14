#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "shared_defs.h"
#include "ready_queue.h"

int init_ready_queue(struct ready_queue** rq)
{
    *rq = malloc(sizeof(struct ready_queue));
    (*rq)->head = NULL;
    (*rq)->tail = NULL;
    (*rq)->size = 0;
    if (pthread_mutex_init(&lock, NULL) == 0)
    {
        return 0;
    }
    return 1;
}

void insert(struct ready_queue *rq, struct thread_node *t)
{
    pthread_mutex_lock(&lock);
    if (rq->head != NULL) 
    {
        rq->tail->next = t;
        rq->tail = rq->tail->next;
    }
    else
    {
        rq->head = t;
        rq->tail = rq->head;
    }
    rq->size++;
    printf("inserted.%d\n", t->t_index);
    
    pthread_mutex_unlock(&lock);
}

void remove_node(ready_queue *rq, int index) {
    printf("beginning remove_node index: %d\n", index);
    thread_node* head = rq->head;
    if (head == NULL) {
        return;
    }
    else if (head->t_index == index) {
        //free(rq->head);
        rq->head = NULL;
        rq->tail = NULL;
        rq->size--;
        printf("deleted\n");
    }
    else {
        while(head->next != NULL) {
            if (head->next->t_index == index) {
                thread_node* temp = head->next->next; 
                //free(head->next);
                head->next = temp;
                rq->size--;
                printf("deleted\n");
                return;
            }
            head = head->next;
        }
    }
}

thread_node* find_shortest_burst(ready_queue* rq) {
	thread_node* cur = rq->head;
	thread_node* min_node;
	int min = cur->pcb.next_burst;

	while (cur != NULL) {
		int burst = cur->pcb.next_burst;
		if (burst < min) {
			min = burst;
			min_node = cur;
		}
		cur = cur->next;
	}

	remove_node(rq, min_node->t_index);
	return min_node;
}

thread_node* dequeue(ready_queue *rq){
	if (rq == NULL || rq->head == NULL) {
		return NULL;
	}

	thread_node* ret = rq->head;
	if (rq->head->next == NULL) {
		rq->head = NULL;
		rq->tail = NULL;
		rq->size = 0;
	} else {
		rq->head = rq->head->next;
		rq->size--;
	}

	return ret;
}

// FCFS, SJF, or RR









