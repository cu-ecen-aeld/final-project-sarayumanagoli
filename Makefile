#Executable

all: helloworld

helloworld: HelloWorld/helloworld.c
	gcc -g -Wall -Werror HelloWorld/helloworld.c -o helloworld

#make clean
clean:
	rm -rf helloworld
