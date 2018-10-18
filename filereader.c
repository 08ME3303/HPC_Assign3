#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

void main(int argc, char**argv){

	printf("%d\n", argc);
	FILE * fptr;
	double start_prog = omp_get_wtime();
	fptr = fopen("./test_data/cell_e5", "r");
	
	//get number of coordintes in the file
	long SIZE = 0;
   	char ch;
  	while ((ch = fgetc(fptr)) != EOF){
       if (ch == '\n' || ch == ' ')
           SIZE++;
   	}
   	
   	long ROWS = SIZE/3;
   	printf("size %ld\t, rows %ld\n", SIZE, ROWS);
   	//get size of file
   	rewind(fptr);
   	fseek(fptr, 0, SEEK_END);
	long file_size = ftell(fptr);
	fseek(fptr, 0, SEEK_SET);
   	
   	//read contents of file
	char * file_buffer = (char*) malloc(sizeof(char) * file_size);
	size_t result = fread ( file_buffer, 1, file_size, fptr );
	
	float * x = (float*)malloc(sizeof(float)*ROWS);
	float * y = (float*)malloc(sizeof(float)*ROWS);
	float * z = (float*)malloc(sizeof(float)*ROWS);
	
	char char_buff[10];
	float rows[3] = {0.0, 0.0, 0.0};
	int row_id = 0, column = 0, char_id = 0;
	char c1 = ' ', c2 = '\n';
	
	for ( int loc = 0; loc < file_size; loc++){
		if (file_buffer[loc] == c1){
			rows[column] = atof(char_buff);
			column++;
			char_id = 0;
			}
		else{
			if (file_buffer[loc] == c2){
				rows[column] = atof(char_buff);
				column = 0;
				x[row_id] = rows[0];
				y[row_id] = rows[1];
				z[row_id] = rows[2];
				row_id++;
				char_id = 0;
				}
			else{
				char_buff[char_id] = file_buffer[loc];
				char_id++;
				}
			}
		}
		
	free(x);
	free(y);
	free(z);
	}
