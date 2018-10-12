#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>

#define nTHREADS 1
	
void main(){

	//opening file
	FILE * fptr;
	double start_prog = omp_get_wtime();
	fptr = fopen("./test_data/cell_e4", "r");
	
	//get number of coordintes in the file
	int SIZE = 0;
   	char ch;
  	while ((ch = fgetc(fptr)) != EOF){
       if (ch == '\n' || ch == ' ')
           SIZE++;
   	}
   	
   	int ROWS = SIZE/3;
   	
   	//get size of file
   	rewind(fptr);
   	fseek(fptr, 0, SEEK_END);
	long file_size = ftell(fptr);
	fseek(fptr, 0, SEEK_SET);
   	
   	//read contents of file
	char * file_buffer = (char*) malloc(sizeof(char) * file_size);
	size_t result = fread ( file_buffer, 1, file_size, fptr );
	
	//array of points
	double * x = (double*)malloc(sizeof(double)*ROWS);
	double * y = (double*)malloc(sizeof(double)*ROWS);
	double * z = (double*)malloc(sizeof(double)*ROWS);
	
	char char_buff[10];
	double rows[3] = {0.0, 0.0, 0.0};
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
	int n = (ROWS * (ROWS-1))/2;
	double distance = 0.0;
	int * possibilities = (int*)malloc(sizeof(int) * 3465);

	
	omp_set_num_threads(nTHREADS);
	double start_init = omp_get_wtime();
	#pragma omp parallel for
	for ( int x = 0; x<3465; x++){
		possibilities[x] = 0;
		}
		
	printf("possibilities init time: %lf \n", omp_get_wtime()-start_init);
	
	double start_distance = omp_get_wtime();
	int i, j;
	
	/*omp_set_num_threads(nTHREADS);
	#pragma omp parallel for //shared (possibilities)
	for ( i = 0; i<ROWS; i++ ){
		omp_set_num_threads(nTHREADS);
		#pragma omp parallel for //shared (possibilities)
		for( j = i+1; j <ROWS; j++){
		
			distance = sqrt(((x[i]-x[j]) * (x[i]-x[j])) + ((y[i]-y[j]) * (y[i]-y[j])) + ((z[i]-z[j])*(z[i]-z[j]))); 
			possibilities[(int)(distance*100)]++;
			
			}
		}*/
		
	//approach 2
	double * xpairs = (double *) malloc(sizeof(double) * n);
	//double * ypairs = (double *) malloc(sizeof(double) * n);
	//double * zpairs = (double *) malloc(sizeof(double) * n);
	
	printf("length of items %d\n", n);
	
	int m,l, id = 0;
	printf("%d\n", ROWS);
	
	omp_set_num_threads(nTHREADS);
	#pragma omp parallel for
	for ( m = 0; m <ROWS; m++){
		#pragma omp parallel for
		for( l = m+1; l <ROWS; l++){
			xpairs[id++] = (x[m]-x[l])*(x[m]-x[l]) + (y[m]-y[l])*(y[m]-y[l]) + (z[m]-z[l])*(z[m]-z[l]);
			}
		}
	

	omp_set_num_threads(nTHREADS);
	#pragma omp parallel for
	for ( m = 0; m < id; m++){
		distance = sqrt(xpairs[m]);
		possibilities[(int)(distance*100)]++;
		}
	
		
	printf("Distance calculating time: %lf \n", omp_get_wtime()-start_distance);
	
	omp_set_num_threads(nTHREADS);
	#pragma omp parallel for
	for ( int i = 0; i < 3465; i++){
		if(possibilities[i] != 0){
			//printf("%.2lf,%d \n", i/100.0, possibilities[i]);
			}
		}
	printf("Total program time: %lf\n", omp_get_wtime()-start_prog);
	
	free(file_buffer);
	free(x);
	free(y);
	free(z);
	}
	
	
