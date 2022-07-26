EXEC = server client

all: EXEC

EXEC:
	gcc -Wall server.c queue.c parsearg.c utils.c databaseManipulation.c -lrt -pthread -o server
	gcc -Wall client.c utils.c parseArgClient.c -lrt -pthread -o client

clean:
	rm -rf $(EXEC)
