CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build: clean
	gcc $(CFLAGS) argReader.c pathReader.c main.c -o myls


run: build
	ls -v1ilR ..
	./myls ..
	./myls -i ..
	./myls -l ..
	./myls -R ..
	./myls -il ..
	./myls -iR ..
	./myls -lR ..
	./myls -ilR ..


valgrind: build
	valgrind --leak-check=full --show-leak-kinds=all ./myls -ilR / > out.txt

clean:
	rm -f myls
