all: systemsim

systemsim: main.c ready_queue.c
	gcc -Wall -g main.c ready_queue.c -o out -lpthread -lm
clean:
	rm -fr out *~
