
//
// Created by Dilay Yiğit on 25.02.2022.
//
#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "shareddefs.h"

mqd_t sendQ;
mqd_t receiveQ;
mqd_t childQ;

void init_queues() {

    receiveQ = mq_open(MQNAME_CL, O_RDONLY | O_CREAT, 0666, NULL);
    sendQ = mq_open(MQNAME_SV, O_RDWR | O_CREAT, 0666, NULL);
    childQ = mq_open(MQNAME_CH, O_RDONLY | O_CREAT, 0666, NULL);

    if (receiveQ == -1 || sendQ == -1 || childQ == -1) {
        perror("ERROR: Server cannot open message queue!\n");
        exit(1);
    }

    printf("Server: Send queue opened. ID: %d\n", (int) sendQ);
    printf("Server: Receive queue opened. ID: %d\n", (int) receiveQ);
    printf("Server: Child queue opened. ID: %d\n", (int) childQ);
}

struct clientInfo* wait_for_request() {
	struct mq_attr mq_attr;
	mq_getattr(receiveQ, &mq_attr);
	int buflen = mq_attr.mq_msgsize;
	char *bufptr = (char *) malloc(buflen);

    int n = mq_receive(receiveQ, (char *) bufptr, buflen, NULL);
    if (n == -1) {
        perror("ERROR: Server cannot receive message queue!\n");
        exit(1);
    }
    struct clientInfo* ptr = (struct clientInfo*) bufptr;
    return ptr;
}

struct childResponse* wait_for_child_message() {
	struct mq_attr mq_attr;
	mq_getattr(childQ, &mq_attr);
	int buflen = mq_attr.mq_msgsize;
	char *bufptr = (char *) malloc(buflen);

    int n = mq_receive(childQ, (char *) bufptr, buflen, NULL);
    if (n == -1) {
        perror("ERROR: Server cannot receive message queue!\n");
        exit(1);
    }
    struct childResponse* ptr = (struct childResponse*) bufptr;
    return ptr;
}


void send_response_to_client(struct responseItem *response_ptr) {
	int n = mq_send(sendQ, (char *) response_ptr, sizeof(struct responseItem), 0);
	if (n == -1) {
	  perror("ERROR: Sending message queue is failed!\n");
	  exit(1);
	}
}

struct responseItem* generateIntervals(struct clientInfo* param) {
	struct responseItem* intervals = malloc(sizeof(struct responseItem) * param->intervalCount);
	for (int i = 0; i < param->intervalCount; i++) {
		struct responseItem item;
		item.intervalIndex = i;
		item.intervalStart = param->intervalStart + (param->intervalWidth * i);
		item.intervalEnd = item.intervalStart + param->intervalWidth;
		item.freq = 0;
		intervals[i] = item;
	}
	return intervals;
}

void calculateHist(const char* fileName, struct clientInfo* param) {
	struct responseItem* intervals = generateIntervals(param);
	int m;
	mqd_t queue = mq_open(MQNAME_CH, O_RDWR);
	    if (queue == -1) {
        perror("ERROR: Child Process cannot open message queue!\n");
        exit(1);
    }
    pid_t myid = getpid();
    
    FILE* fp = fopen(fileName, "r");
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    if ( fp == NULL ) {  
    	printf("Unable to open file."); 
    	exit(1);
	}  
	int num;
	while (!feof (fp))
	{  
		fscanf (fp, "%d", &num);
		if (num < param->intervalStart) {
			continue;
		}
		num = num - param->intervalStart;
		int interval = num / param->intervalWidth;
		if (interval > param->intervalCount) {
			continue;
		}
		intervals[interval].freq++;      
	}
	
	struct childResponse result;
	result.pid = getpid();
    
    for (int i = 0; i < param->intervalCount; i++) {
		result.response = intervals[i];
		m = mq_send(queue, (char *) &result, sizeof(struct childResponse), 0);
        if (m == -1) {
            perror("ERROR: Sending message queue is failed!\n");
            exit(1);
        }
        printf("child send: %d %d %d %d\n", result.response.intervalStart, result.response.intervalEnd, result.response.intervalIndex, result.response.freq);
    }

}

int main(int argc, char** argv) {

	init_queues();

    struct clientInfo* params;
    struct childResponse* child_response;
    struct responseItem* intervals = generateIntervals(params);
   
    pid_t childid;
    
    int numOfFile = atoi(argv[1]);
    char *fileNames[10];
	for (int i = 0; i < numOfFile; i++) {
		fileNames[i] = (char *) argv[i+2];
	}

	params = wait_for_request();
    printf("param1: %d\n", params->intervalCount);
	for (int i = 0; i < numOfFile; i++) {
		childid = fork();
		if (childid < 0) {
			perror("ERROR: Child process cannot created!\n");
			exit(1);
		}
		if (childid == 0) {
			// this part executed by child process
			calculateHist(fileNames[i], params);
			exit(0);
		}
	}
    sleep(5);
	int expected_message_count = numOfFile * params->intervalCount;
	while (expected_message_count-- > 0) {
		child_response = wait_for_child_message();
		int intervalIndex = (child_response->response).intervalIndex;
		intervals[intervalIndex].freq += (child_response->response).freq;
    }
    for (int i = 0; i < params->intervalCount; i++) {
    	send_response_to_client(&intervals[i]);
    }

	exit(0);
}
