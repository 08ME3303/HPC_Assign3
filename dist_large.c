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
	(ROWS<=10000)?(blocks = 1):(blocks = 1000);
		
	long long int num_ops = 0;	
	long long int k = 0;
	int i,j, ind;
	float r = 0.0, x1, y1, z1;
	int distance;
	int flag;
	float * xyz;
	int iblock, imax, jblock, jmax, ix, jx, kx, mx;
	float * temp = (float*) calloc(n/blocks, sizeof(float));
	float temporary;

	for (iblock=0; iblock < ROWS; iblock+=blocks) {
 		imax = iblock + blocks < ROWS ? iblock + blocks : ROWS-1;

	    for (ix=iblock; ix < imax; ix++){
	    	x1 = x[ix];
	    	y1 = y[ix];
			z1 = z[ix];
  			for (jx=ix+1; jx < ROWS; jx++){
  				temporary = (((x1-x[jx]) * (x1-x[jx])) + ((y1-y[jx]) * (y1-y[jx])) + ((z1-z[jx])*(z1-z[jx])));
  				distance = 100/(_mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(temporary))));
				possibilities[distance]++;
  				}
  			}
		}

	for ( int i = 0; i < 3465; i++){
		if(possibilities[i] != 0){
			//dx += possibilities[i];
			printf("%.2lf,%d \n", i/100.0, possibilities[i]);
			}
		}
		
	printf("Total program time: %lf\n", omp_get_wtime()-start_prog);
	
	free(file_buffer);
	free(temp);
	free(x);
	free(y);
	free(z);
	}
