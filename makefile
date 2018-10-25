CC=gcc
CFLAGS=-c -fopenmp -O3

.PHONY : clean

cell_distance: cell_distance_2.o
	$(CC) -o $@ $< -lm -fopenmp
	
cell_distance_2.o: cell_distance_2.c
	$(CC) $(CFLAGS) cell_distance_2.c
	
clean :
	rm -rf cell_distance_2.o cell_distance_2 extracted reports distances
