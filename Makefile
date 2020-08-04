CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build:
	gcc $(CFLAGS) argReader.c pathReader.c main.c -o myls


run: build
	ls -v1ilR /home/crystalloid/cmpt300 > 1.txt
	./myls -ilR /home/crystalloid/cmpt300 > 2.txt

valgrind: build
	valgrind --leak-check=full --show-leak-kinds=all ./myls -ilR /home/crystalloid/cmpt300 > 2.txt

clean:
	rm -f myls
