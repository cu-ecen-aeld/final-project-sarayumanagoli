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

producer: SocketClient_Integrated/producer.c
	$(CC) $(CFLAGS) $(INCLUDES) SocketClient_Integrated/producer.c -o producer $(LDFLAGS)

consumer: SocketClient_Integrated/consumer.c
	$(CC) $(CFLAGS) $(INCLUDES) SocketClient_Integrated/consumer.c -o consumer $(LDFLAGS)


servertest: SocketServer_Integrated/server.c
	$(CC) $(CFLAGS) SocketServer_Integrated/server.c -o servertest $(LDFLAGS)

clienttest: SocketClient_Integrated/client.c
	$(CC) $(CFLAGS) SocketClient_Integrated/client.c -o clienttest $(LDFLAGS)

receiver: SocketServer_Integrated/receiver.c
	$(CC) $(CFLAGS) SocketServer_Integrated/receiver.c SocketServer_Integrated/gpio.c -o receiver $(LDFLAGS)

#make clean
clean:
	rm -rf producer consumer servertest clienttest receiver
