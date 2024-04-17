CC = g++
CFLAGS = -Wall -Wextra -g
LDLIBS = -lsfml-graphics -lsfml-system -lsfml-window

compile: clear
	$(CC) $(CFLAGS) main2.cpp $(LDLIBS)

clear:
	clear
