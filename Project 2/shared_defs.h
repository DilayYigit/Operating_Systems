#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

typedef struct PCB {
	pid_t pid;
	pthread_t tid;
	char* state;
	int next_burst;
	int remain_burst;
	int executed_burst;
	int time_spent;
	int device1;
	int device2;
	int arrival_time;
	int finish_time;
	int total_execution;
} PCB;

typedef struct thread_node
{
    int t_index;   
    pthread_attr_t t_attr;
    pthread_cond_t t_cond; 
    struct PCB pcb;
	struct thread_node* next;
} thread_node;

typedef struct ready_queue {
	struct thread_node* head;
	struct thread_node* tail;
    int size;
} ready_queue;

#endif