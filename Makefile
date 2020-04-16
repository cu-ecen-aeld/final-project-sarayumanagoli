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

all: helloworld sharedmem server client servertest clienttest TMP102 gassensor led adcread

helloworld: HelloWorld/helloworld.c
	$(CC) $(CFLAGS) $(INCLUDES) HelloWorld/helloworld.c -o helloworld

sharedmem: SharedMemory/sharedmem.c
	$(CC) $(CFLAGS) $(INCLUDES) SharedMemory/sharedmem.c -o sharedmem $(LDFLAGS)

server: SocketServer/server.c
	$(CC) $(CFLAGS) SocketServer/server.c -o server $(LDFLAGS)

client: SocketClient/client.c
	$(CC) $(CFLAGS) SocketClient/client.c -o client $(LDFLAGS)

servertest: SocketTest/server.c
	$(CC) $(CFLAGS) SocketTest/server.c -o servertest $(LDFLAGS)

clienttest: SocketTest/client.c
	$(CC) $(CFLAGS) SocketTest/client.c -o clienttest $(LDFLAGS)

TMP102: Temperature_Sensor/TMP102.c
	$(CC) $(CFLAGS) Temperature_Sensor/TMP102.c -o TMP102 $(LDFLAGS)

gassensor: GasSensor/gassensor.c
	$(CC) $(CFLAGS) GasSensor/gassensor.c -o gassensor $(LDFLAGS)

adcread: GasSensor/ADC_Read.c
	$(CC) $(CFLAGS) GasSensor/ADC_Read.c -o adcread $(LDFLAGS)

led: LED/led.c
	$(CC) $(CFLAGS) LED/led.c -o led $(LDFLAGS)

#make clean
clean:
	rm -rf helloworld sharedmem server client servertest clienttest TMP102 gassensor led adcread
