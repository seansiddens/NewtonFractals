CC = clang
CFLAGS = -Wall
LFLAGS = -lm

all: newton

newton: newton.o
	$(CC) -o newton newton.o $(LFLAGS)

newton.o: newton.c
	$(CC) $(CFLAGS) -c newton.c

clean: 
	rm -f newton newton.o

format: 
	clang-format -i -style=file *.[ch]

scan-build: clean
	scan-build make

