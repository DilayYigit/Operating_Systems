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
int N;
char* algo;
char* burst_dist;
int burst_length;
int min_burst;
int max_burst;
int max_process;
int all_process;
int output_mode;
int t1;
int t2;
float pg;
float p0;
float p1;
float p2;
char* quantum_str;
int quantum;
struct ready_queue* rq;
pthread_mutex_t sleep_lock;
int i = 0;
struct timeval simulator_start_time;

pthread_mutex_t sch_cond;
pthread_mutex_t sch_mutex;

pthread_mutex_t proc_cond;
pthread_mutex_t proc_mutex;

pthread_mutex_t dev1_cond;
pthread_mutex_t dev1_mutex;

pthread_mutex_t dev2_cond;
pthread_mutex_t dev2_mutex;

pthread_t chosen_id;


float flip_probability() {
	float a = 1.0;

	return ((float)rand()/(float)(RAND_MAX)) * a;
}

int elapsed_time() {
	struct timeval cur_time;
	gettimeofday(&cur_time, NULL);
	int cur_time_in_ms = (int)cur_time.tv_sec * 1000 + (int)cur_time.tv_usec;
	int start_time_in_ms = (int)simulator_start_time.tv_sec * 1000 + (int)simulator_start_time.tv_usec;

	return cur_time_in_ms - start_time_in_ms;
}

void* process_scheduler(void* args) {
	pthread_mutex_lock(&sch_mutex);

	while(1) {
		pthread_cond_wait(&sch_cond, &sch_mutex);

		// check if scheduling is necessary

		// choose a thread based on the algorithm
		thread_node* chosen = NULL;
		if (strcmp(algo, "FCFS")) {
			chosen = dequeue(rq);
		} else if (strcmp(algo, "SJF")) {
			chosen = find_shortest_burst(rq);
		} else if (strcmp(algo, "RR")) {
			chosen = dequeue(rq);
		}

		chosen_id = chosen->pcb.tid;
		pthread_cond_broadcast(&proc_cond);
	}

}


void* process_thread(void* args) {
	thread_node* self = (thread_node*)args;
	PCB pcb = self->pcb;

	pcb.arrival_time = elapsed_time();
	pcb.state = "READY";
	insert(rq, self);

	pthread_mutex_lock(&proc_mutex);
	while(1) {
		pthread_cond_wait(&proc_cond, &proc_mutex);

		// check if scheduler meant me
		if (pcb.tid == chosen_id) {
			pcb.state = "RUNNING";

			// remove myself from ready_queue
			remove_node(rq, self->t_index);
			pcb.time_spent = elapsed_time() - pcb.arrival_time;

			// run on cpu
			usleep(pcb.next_burst);

			// update stats
			pcb.executed_burst++;
			pcb.total_execution += pcb.next_burst;
			pcb.finish_time = elapsed_time();

			// terminate or device1 or device2
			float prob = flip_probability();
			if (prob <= p0) {
				// terminate
				return NULL;
			} else if (prob > p0 && prob <= p1) {
				// device 1

			} else {
				// device 2
			}
		}

	}

	pthread_exit(NULL);
}

void thread_creation(pthread_t tid, pid_t pid, thread_node* t_node) {
	struct PCB pcb_node;
	t_node->t_index = i + 1;
	pcb_node.pid = pid;
	pcb_node.tid = i + 1;
	pcb_node.state = "READY";
	if (strcmp(burst_dist, "fixed") == 0) {
		pcb_node.next_burst = burst_length;
	}
	else if (strcmp(burst_dist, "uniform") == 0) {
		pcb_node.next_burst = rand() % (max_burst - min_burst + 1) + min_burst;
	}
	else if (strcmp(burst_dist, "exponential") == 0) {
		double lambda = 1 / (double)(burst_length);
		pcb_node.next_burst = -1 * log(1 - (rand() / (RAND_MAX + 1.0))) / lambda;
	}
	if (strcmp(algo,"RR")) {
		pcb_node.remain_burst = burst_length;

	}
	pcb_node.executed_burst = 0;
	pcb_node.time_spent = 0;
	pcb_node.device1 = t1;
	pcb_node.device2 = t2;
	pcb_node.total_execution = 0; 
	t_node->pcb = pcb_node;	
}


void* process_generator(void* args) {
	int initial_process_count = max_process < 10 ? max_process : 10;
	int total_process_count = initial_process_count;
	for (; i < initial_process_count; i++) {
		thread_node t_node;
		pthread_t tid;
		pid_t pid = getpid();
		thread_creation(tid, pid, &t_node);
		pthread_create(&tid, NULL, process_thread, &t_node);
		
	}
	
	while(all_process >= total_process_count) {
		usleep(5000);
		float probability = flip_probability();
		if (max_process > rq->size && probability <= pg) {
			pthread_t t;
			pid_t p = getpid();
			thread_node t_node2;
			thread_creation(t,p, &t_node2);
			pthread_create(&t, NULL, process_thread, &t_node2);
			total_process_count++;
			printf("process generated %d\n", rq->size);
			i++;
		}

	}
}



void init_params(int argc, char *argv[]) {
	if (argc != 16) {
		printf("Input is wrong! \n");
		exit(1);
	}
	else {
		algo = argv[1];
		if (strcmp(algo,"FCFS") == 0 || strcmp(algo,"SJF") == 0) {
			quantum_str = argv[2];
		}
		else if (strcmp(algo,"RR") == 0) {
			quantum = atoi(argv[2]);
		}
		t1 = atoi(argv[3]);
		t2 = atoi(argv[4]); 
		burst_dist = argv[5];
		burst_length = atoi(argv[6]);
		min_burst = atoi(argv[7]);
		max_burst = atoi(argv[8]);
		p0 = atof(argv[9]);
		p1 = atof(argv[10]);
		p2 = atof(argv[11]);
		pg = atof(argv[12]);
		max_process = atoi(argv[13]);
		all_process = atoi(argv[14]);
		output_mode = atoi(argv[15]);
	}
	
}

int main(int argc, char *argv[]) {
	gettimeofday(&simulator_start_time, NULL);

	pthread_mutex_init(&sleep_lock, NULL);
	init_params(argc, argv);
	init_ready_queue(&rq);

	// create the process_generator thread
	pthread_t pg_t;
	pthread_create(&pg_t, NULL, process_generator, NULL);

	// create the scheduler thread
	pthread_t sc_t;
	pthread_create(&sc_t, NULL, process_scheduler, NULL);

	return 0;
}
