CC = gcc
CFLAGS = -fopenmp -O3 
OBJS = cell_distance.o
LIBS = -lm

.PHONY : clean

cell_distance : $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LIBS)
	
cell_distance.o : cell_distance.c

clean : 
	rm -f $(OBJS) cell_distance
