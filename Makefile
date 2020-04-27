#Executable

ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt
endif

all: producer consumer servertest clienttest receiver

producer: SharedMemory_Integrated/producer.c
	$(CC) $(CFLAGS) $(INCLUDES) SharedMemory_Integrated/producer.c -o producer $(LDFLAGS)

consumer: SharedMemory_Integrated/consumer.c
	$(CC) $(CFLAGS) $(INCLUDES) SharedMemory_Integrated/consumer.c -o consumer $(LDFLAGS)


servertest: SocketServer_Integrated/server.c
	$(CC) $(CFLAGS) SocketServer_Integrated/server.c -o servertest $(LDFLAGS)

clienttest: SocketTest/client.c
	$(CC) $(CFLAGS) SocketTest/client.c -o clienttest $(LDFLAGS)

receiver: SocketServer_Integrated/receiver.c
	$(CC) $(CFLAGS) SocketServer_Integrated/receiver.c SocketServer_Integrated/gpio.c -o receiver $(LDFLAGS)

#make clean
clean:
	rm -rf producer consumer servertest clienttest receiver
