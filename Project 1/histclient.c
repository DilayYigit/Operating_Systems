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



int main(int argc, char** argv) {

	mqd_t sendQ;
    mqd_t receiveQ;
    struct responseItem* itemptr;
    struct clientInfo params;
    int n;
    char *bufptr;
    int buflen;
    struct mq_attr mq_attr;

    sendQ = mq_open(MQNAME_CL, O_RDWR);
    receiveQ = mq_open(MQNAME_SV, O_RDONLY);

    if (sendQ == -1 || receiveQ == -1) {
        perror("ERROR: Client cannot open message queue!\n");
        exit(1);
    }
    printf("Client: Send queue opened. ID: %d\n", (int) sendQ);
    printf("Client: Receive queue opened. ID: %d\n", (int) receiveQ);

	int count, width, start;
	printf("enter count: \n");
	scanf("%d", &count);
	printf("enter width: \n");
	scanf("%d", &width);
	printf("enter start: \n");
	scanf("%d", &start);

	params.intervalCount = count;
	params.intervalWidth = width;
	params.intervalStart = start;
    n = mq_send(sendQ, (char *) &params, sizeof(struct clientInfo), 0);
    if (n == -1) {
        perror("ERROR: Sending message queue is failed!\n");
        exit(1);
    }
    mq_getattr(receiveQ, &mq_attr);
    buflen = mq_attr.mq_msgsize;
    bufptr = (char *) malloc(buflen);
    
    for (int i = 0; i < params.intervalCount; i++) {
		n = mq_receive(receiveQ, (char *) bufptr, buflen, NULL);
		if (n == -1) {
		    perror("mq_receive failed\n");
		    exit(1);
		}

		itemptr = (struct responseItem* ) bufptr;
		printf("[%d, %d): %d\n", itemptr->intervalStart, itemptr->intervalEnd, itemptr->freq);

	}
    printf("\n");
        
    free(bufptr);
    mq_close(receiveQ);
    mq_close(sendQ);
    sleep(1);

}

