#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include <xmmintrin.h>
	
void main(int argc, char** argv){
	
	int nTHREADS;
	if (argc == 2){
		nTHREADS=strtol((strtok(argv[1], "-t")),NULL,10); 
		}
	else{
		printf("Incorrect number of arguments\n");
		return;
		}
	
	//opening file
	FILE * fptr;
	double start_prog = omp_get_wtime();
	fptr = fopen("./cells", "r");
	
	//get number of coordintes in the file
	long SIZE = 0;
   	char ch;
  	while ((ch = fgetc(fptr)) != EOF){
       if (ch == '\n' || ch == ' ')
           SIZE++;
   	}
   	
   	long ROWS = SIZE/3;
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
	char c1 = ' ', c2 = '\n';
	double start = omp_get_wtime();

	//parsing file for necessary data	

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
	
	//printf("File parsing time taken: %lf \n", omp_get_wtime()-start);	
	fclose(fptr);
		
	//distance calculator
	long long int n = (ROWS * (ROWS-1))/2;
	int distance = 0;
	int * possibilities = (int*)calloc(3465, sizeof(int));
		
	//long long int num_ops = 0;	
	int i,j;
	float x1, y1, z1;

	float temporary;
	double start_distance1 = omp_get_wtime();		
	
	omp_set_num_threads(nTHREADS);
	#pragma omp parallel for schedule(dynamic, 10) reduction(+:possibilities[:3465])
	for ( i = 0; i < ROWS; i++){
		for ( j = i+1; j < ROWS; j++ ) {
			temporary = (((x[i]-x[j]) * (x[i]-x[j])) + ((y[i]-y[j]) * (y[i]-y[j])) + ((z[i]-z[j])*(z[i]-z[j])));
			//distance = 100*sqrt(temporary);
			distance = 100/(_mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(temporary))));
			possibilities[distance]++;
			}
		}	
		
	double end_distance1 = omp_get_wtime();
	//num_ops = 0;
	
	/*for ( int i = 0; i < 3465; i++){
		if(possibilities[i] > 0){
			//num_ops+=possibilities[i];
			printf("%.2lf %d \n", i/100.0, possibilities[i]);
			}
		}*/
		
		
	for ( int i = 0; i < 3465; i++){
		//num_ops+=possibilities[i];
		printf("%.2lf %d \n", i/100.0, possibilities[i]);
		}
		
	// Debugging and benchmarking outputs
	//printf("%lld\n", num_ops);
	printf("Time for distance calculations: %lf\n", end_distance1-start_distance1);
	printf("Total program time: %lf\n", omp_get_wtime()-start_prog);
	
	free(file_buffer);
	free(x);
	free(y);
	free(z);
	}
