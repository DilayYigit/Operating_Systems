//
// Created by dilay on 3/2/22.
//

#ifndef PROJECT1_SHAREDDEFS_H
#define PROJECT1_SHAREDDEFS_H

struct responseItem {
	int intervalStart;
    int intervalEnd;
    int freq;
    int intervalIndex;
};

struct childResponse {
	struct responseItem response;
	int pid;
};

struct clientInfo {
    int intervalCount;
    int intervalWidth;
    int intervalStart;
};

#define MQNAME_CL "/client.queue"
#define MQNAME_SV "/server.queue"
#define MQNAME_CH "/child.queue"

#endif //PROJECT1_SHAREDDEFS_H

