# At the end:
# if we write make childP, we will have child process executables
# or make thread, we will have thread executables
all: server client

client: histclient.c
	gcc -Wall -o histclient histclient.c -lrt

server: histserver.c
	gcc -Wall -o histserver histserver.c -lrt

clean:
	rm -fr *~ histclient histserver