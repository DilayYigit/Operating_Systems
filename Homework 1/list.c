#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <sys/time.h> 
struct Node {
	int data;
	struct Node* next; 
};
int main() {
	struct timeval start, end;
	gettimeofday(&start, NULL);
	struct Node* head = (struct Node*)malloc(sizeof(struct Node));
	head->data = rand() % 100 + 1; 
	head->next = NULL;
	struct Node* cur = head; 
	for(int i = 0; i < 10000; i++) {
		cur->next = malloc(sizeof(struct Node)); 
		cur = cur->next;
		cur->data = rand() % 100 + 1;
	}
	gettimeofday(&end, NULL);
	printf("%ld\n", (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec));
	return 0;
}