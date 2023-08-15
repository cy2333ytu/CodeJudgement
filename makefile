CC = g++
CFLAGS = -std=c++17 -Wall -Wextra

OBJS = SimpleOJ.o main.o

all: simpleoj

simpleoj: $(OBJS)
	$(CC) $(CFLAGS) -o simpleoj $(OBJS)

SimpleOJ.o: SimpleOJ.cpp SimpleOJ.h
	$(CC) $(CFLAGS) -c SimpleOJ.cpp

main.o: main.cpp SimpleOJ.h
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -f simpleoj $(OBJS)
