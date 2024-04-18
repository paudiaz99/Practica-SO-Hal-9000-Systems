CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -pthread

all: Poole/poole Bowman/bowman Discovery/discovery

TextUtilLib.o: Libraries/TextUtilLib.c Libraries/TextUtilLib.h
	$(CC) $(CFLAGS) -c Libraries/TextUtilLib.c -o TextUtilLib.o

Comunication.o: Libraries/Comunication.c Libraries/Comunication.h
	$(CC) $(CFLAGS) -c Libraries/Comunication.c -o Comunication.o

Sockets.o: Libraries/Sockets.c Libraries/Sockets.h
	$(CC) $(CFLAGS) -c Libraries/Sockets.c -o Sockets.o

UniversalLinkedList.o: Libraries/UniversalLinkedList.c Libraries/UniversalLinkedList.h
	$(CC) $(CFLAGS) -c Libraries/UniversalLinkedList.c -o UniversalLinkedList.o

userInterface.o: Libraries/userInterface.c Libraries/userInterface.h
	$(CC) $(CFLAGS) -c Libraries/userInterface.c -o userInterface.o

pooleInterface.o: Libraries/pooleInterface.c Libraries/pooleInterface.h
	$(CC) $(CFLAGS) -c Libraries/pooleInterface.c -o pooleInterface.o

Poole/poole: TextUtilLib.o Sockets.o Comunication.o UniversalLinkedList.o pooleInterface.o Libraries/Config.h Libraries/semaphore_v2.h Poole/Poole.c
	$(CC) $(CFLAGS) -o Poole/poole TextUtilLib.o Sockets.o Comunication.o UniversalLinkedList.o pooleInterface.o Poole/Poole.c $(LIBS)

Bowman/bowman: TextUtilLib.o Sockets.o Comunication.o userInterface.o UniversalLinkedList.o Libraries/Config.h Bowman/Bowman.c
	$(CC) $(CFLAGS) -o Bowman/bowman TextUtilLib.o Sockets.o Comunication.o userInterface.o UniversalLinkedList.o Bowman/Bowman.c $(LIBS)

Discovery/discovery: TextUtilLib.o Sockets.o Comunication.o UniversalLinkedList.o Libraries/Config.h Discovery/Discovery.c
	$(CC) $(CFLAGS) -o Discovery/discovery TextUtilLib.o Sockets.o Comunication.o UniversalLinkedList.o Discovery/Discovery.c $(LIBS)

clean:
	rm -f Poole/poole Bowman/bowman Discovery/discovery *.o
