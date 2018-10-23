CC=gcc
CFLAGS=-c -fopenmp -O3

.PHONY : clean

cell_distance: cell_distance.o
	$(CC) -o $@ $< -lm -fopenmp
	
cell_distance.o: cell_distance.c
	$(CC) $(CFLAGS) cell_distance.c
	
clean :
	rm -f cell_distance.o cell_distance
	rm -rf extracted/ reports/ distances/
