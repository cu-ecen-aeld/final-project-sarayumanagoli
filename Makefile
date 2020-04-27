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

all: producer consumer servertest clienttest led sender receiver

producer: SharedMemory_Integrated/producer.c
	$(CC) $(CFLAGS) $(INCLUDES) SharedMemory_Integrated/producer.c -o producer $(LDFLAGS)

consumer: SharedMemory_Integrated/consumer.c
	$(CC) $(CFLAGS) $(INCLUDES) SharedMemory_Integrated/consumer.c -o consumer $(LDFLAGS)


servertest: SocketTest/server.c
	$(CC) $(CFLAGS) SocketTest/server.c -o servertest $(LDFLAGS)

clienttest: SocketTest/client.c
	$(CC) $(CFLAGS) SocketTest/client.c -o clienttest $(LDFLAGS)

led: LED/led.c
	$(CC) $(CFLAGS) LED/led.c -o led $(LDFLAGS)

sender: MessageQueue/sender.c
	$(CC) $(CFLAGS) MessageQueue/sender.c -o sender $(LDFLAGS)

receiver: MessageQueue/receiver.c
	$(CC) $(CFLAGS) MessageQueue/receiver.c -o receiver $(LDFLAGS)

#make clean
clean:
	rm -rf producer consumer servertest clienttest led sender receiver
