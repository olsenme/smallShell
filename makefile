CC=clang
CFLAGS=-g -Wpedantic 
all: olsenme.buildrooms olsenme.adventure
olsenme.buildrooms: olsenme.buildrooms.c
	$(CC) olsenme.buildrooms.c -o olsenme.buildrooms $(CFLAGS)
olsenme.adventure: olsenme.adventure.c
	$(CC) olsenme.adventure.c -o olsenme.adventure $(CFLAGS) -lpthread
clean :
	rm -f *.o olsenme.adventure olsenme.buildrooms
