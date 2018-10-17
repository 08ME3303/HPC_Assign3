#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include <xmmintrin.h>

#define nTHREADS 5

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
	
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
	//int distance = 0;
	int * possibilities = (int*)malloc(sizeof(int) * 3465);
	//float * temp = (float*)malloc(sizeof(float)*n);
	//float temp[n];
	
	omp_set_num_threads(nTHREADS);
	double start_init = omp_get_wtime();
	#pragma omp parallel for
	for ( int xin = 0; xin<3465; xin++){
		possibilities[xin] = 0;
		}
		
	printf("possibilities init time: %lf \n", omp_get_wtime()-start_init);
	
	double start_distance = omp_get_wtime();
	int blocks;
	(ROWS<=1000)?(blocks = 1):(blocks = 1000);
		
	long long int num_ops = 0;	
	long long int k = 0;
	int i,j, ind;
	float r = 0.0, x1, y1, z1;
	int distance;
	int flag;
	float * xyz;
	
	(ROWS<=1e4)? (flag = 1) : (flag = 0); 
	printf("All okay upto here with flag %d\n", flag);
	
	switch(flag){
		case 0:
			printf("Large file\n");
			omp_set_num_threads(nTHREADS);
			#pragma omp parallel for shared(x,y,z)
			for ( k = 0; k < n; k++ ){
				i = k/(ROWS), j = k%(ROWS);
				if (j <= i ) i = ROWS-i-2, j = ROWS-j-1;
				distance = (sqrtf(((x[i]-x[j]) * (x[i]-x[j])) + ((y[i]-y[j]) * (y[i]-y[j])) + ((z[i]-z[j])*(z[i]-z[j]))))*100; 
				#pragma omp critical
				possibilities[distance]++;
				}
		
		case 1:
			xyz = (float*)malloc(sizeof(float)*n);
			for ( i = 0; i < ROWS; i++){
				x1 = x[i]; 
				y1 = y[i];
				z1 = z[i];
				for ( j = i+1; j < ROWS; j++){
					xyz[k] = ((x1 - x[j])*(x1-x[j]) + (y1-y[j])*(y1-y[j]) + (z1 - z[j])*(z1-z[j]));
					k++;
					}
				}
			
			for ( k = 0; k < n; k++ ){
				ind = sqrtf(xyz[k])*100;
				possibilities[ind]++;
				}
			free(xyz);
		}
			
		
	/*for ( k = 0; k < n; k++ ){
		i = k/(ROWS), j = k%(ROWS);
		if (j <= i ) i = ROWS - i -2, j = ROWS-j-1;
		xyz[k] = ((x[i]-x[j]) * (x[i]-x[j])) + ((y[i]-y[j]) * (y[i]-y[j])) + ((z[i]-z[j])*(z[i]-z[j]));
		}*/
	
	//printf("%lf\n", temp[k]);
	
	//omp_set_num_threads(nTHREADS);
	//#pragma omp parallel for shared(x,y,z)
	
	//final printing operation
	for ( int i = 0; i < 3465; i++){
		if(possibilities[i] != 0){
			num_ops += possibilities[i];
			//printf("%.2lf,%d \n", i/100.0, possibilities[i]);
			}
		}
		
	printf("Total distance operations done: %lld, %lf\n", num_ops, (float)num_ops/n);
	printf("Total program time: %lf\n", omp_get_wtime()-start_prog);
	
	free(file_buffer);
	free(x);
	free(y);
	free(z);
	}
	
	
