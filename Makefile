CC=gcc
CFLAGS=-Wall -g

all: oss user_process

oss: oss.o
	$(CC) $(CFLAGS) -o oss oss.o

user_process: user_process.o
	$(CC) $(CFLAGS) -o user_process user_process.o

oss.o: oss.c
	$(CC) $(CFLAGS) -c oss.c

user_process.o: user_process.c
	$(CC) $(CFLAGS) -c user_process.c

clean:
	rm -f *.o oss user_process

