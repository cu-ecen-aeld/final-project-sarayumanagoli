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

all: producer consumer servertest clienttest receiver client_demo server_demo TMP102 producer_demo consumer_demo gassensor

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

producer_demo: Test_Files/SharedMemory_Integrated/producer.c
	$(CC) $(CFLAGS) Test_Files/SharedMemory_Integrated/producer.c -o producer_demo $(LDFLAGS)

consumer_demo: Test_Files/SharedMemory_Integrated/consumer.c
	$(CC) $(CFLAGS) Test_Files/SharedMemory_Integrated/consumer.c -o consumer_demo $(LDFLAGS)


client_demo:	Test_Files/SocketTest/client.c
	$(CC) $(CFLAGS) Test_Files/SocketTest/client.c -o client_demo $(LDFLAGS)

server_demo:	Test_Files/SocketTest/server.c
	$(CC) $(CFLAGS) Test_Files/SocketTest/server.c -o server_demo $(LDFLAGS)

TMP102:	Test_Files/Temperature_Sensor/TMP102.c
	$(CC) $(CFLAGS) Test_Files/Temperature_Sensor/TMP102.c -o TMP102 $(LDFLAGS)

gassensor:	Test_Files/GasSensor/gassensor.c
	$(CC) $(CFLAGS) Test_Files/GasSensor/gassensor.c -o gassensor $(LDFLAGS)

#make clean
clean:
	rm -rf producer consumer servertest clienttest receiver client_demo server_demo TMP102 producer_demo consumer_demo gassensor
