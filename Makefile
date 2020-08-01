CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build:
	gcc $(CFLAGS) main.c -o myls


run: build
	./myls -ilR

valgrind: build
	valgrind --leak-check=full --show-leak-kinds=all ./myls -ilR

clean:
	rm -f myls