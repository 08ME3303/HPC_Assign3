#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>

#define nTHREADS 5
	
void main(){

	//opening file
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
	
	//array of points
	float * x = (float*)malloc(sizeof(float)*ROWS);
	float * y = (float*)malloc(sizeof(float)*ROWS);
	float * z = (float*)malloc(sizeof(float)*ROWS);
	
	char char_buff[10];
	float rows[3] = {0.0, 0.0, 0.0};
	int row_id = 0, column = 0, char_id = 0;
	
	double start = omp_get_wtime();
	//parsing file for necessary data	
	omp_set_num_threads(nTHREADS);
	#pragma parallel for
	for ( int loc = 0; loc < file_size; loc++ ){
		switch ( file_buffer[loc] ) {
			case ' ' :
				rows[column] = atof(char_buff);
				column++;
				char_id = 0;
				
			case '\n' :
				if(column == 2){
					rows[column] = atof(char_buff);
					x[row_id] = rows[0];
					y[row_id] = rows[1];
					z[row_id] = rows[2];
					row_id++;
					column = 0;
					char_id = 0;
					}
				else{
					char_id = 0;
					}
			default : 
				char_buff[char_id] = file_buffer[loc];
				char_id++;
			}
		}
		
	printf("File parsing time taken: %lf \n", omp_get_wtime()-start);
	
	fclose(fptr);
		
	//distance calculator
	long long int n = (ROWS * (ROWS-1))/2;
	printf("n: %lld\n", n);
	int distance = 0;
	int * possibilities = (int*)malloc(sizeof(int) * 3465);

	
	omp_set_num_threads(nTHREADS);
	double start_init = omp_get_wtime();
	#pragma omp parallel for
	for ( int x = 0; x<3465; x++){
		possibilities[x] = 0;
		}
	printf("%d\n", possibilities[3464]);
		
	printf("possibilities init time: %lf \n", omp_get_wtime()-start_init);
	
	double start_distance = omp_get_wtime();
	int i, j, k;
	int blocks;
	(ROWS<=1000)?(blocks = 1):(blocks = 100);
	
	/*for ( k =0; k < blocks; k++ ){
		omp_set_num_threads(nTHREADS);
		#pragma omp parallel for shared(x, y, z, ROWS, k, possibilities) private (i, j)
		for ( i = k*blocks; i<((k*blocks) + (ROWS/blocks)); i++ ){
			for( j = i+1; j <ROWS; j++){
				distance = (sqrtf(((x[i]-x[j]) * (x[i]-x[j])) + ((y[i]-y[j]) * (y[i]-y[j])) + ((z[i]-z[j])*(z[i]-z[j]))))*100; 
				possibilities[distance]++;
				}
			}
		}*/
		
	long long int num_ops = 0;

	//omp_set_num_threads(nTHREADS);
	//#pragma omp parallel for shared(x, y, z, ROWS, k, possibilities) private (i, j)
	for ( i = 0; i < ROWS; i ++){
		omp_set_num_threads(nTHREADS);
		#pragma omp parallel for shared(x,y,z, ROWS, possibilities) private(k,j)
		for ( k = 0; k < blocks; k++){
			for( j = i+ k*blocks; j<((k*blocks) + (ROWS/blocks)); j++){
				distance = (sqrtf(((x[i]-x[j]) * (x[i]-x[j])) + ((y[i]-y[j]) * (y[i]-y[j])) + ((z[i]-z[j])*(z[i]-z[j]))))*100; 
				possibilities[distance]++;
				}
			}
		}

	
		
	printf("Distance calculating time: %lf \n", omp_get_wtime()-start_distance);
	
	for ( int i = 0; i < 3465; i++){
		if(possibilities[i] != 0){
			num_ops += possibilities[i];
			printf("%.2lf,%d \n", i/100.0, possibilities[i]);
			}
		}
		
	printf("Total distance operations done: %lld\n", num_ops);
	printf("Total program time: %lf\n", omp_get_wtime()-start_prog);
	
	free(file_buffer);
	free(x);
	free(y);
	free(z);
	}
	
	
