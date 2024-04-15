CC = g++
CFLAGS = -Wall -Wextra -g
LDLIBS = -lsfml-graphics -lsfml-system -lsfml-window

run: compile
	./a.out

compile: clear
	$(CC) $(CFLAGS) main.cpp $(LDLIBS)

clear:
	clear
