#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>

#define nTHREADS 10

typedef struct points{
	double x;
	double y;
	double z;
	}Point;
	
typedef struct frequency{
	char length[8];
	}Frequency;
	
double odometer(Point p1, Point p2){
	double distance;
	distance = pow(p1.x-p2.x, 2) + pow(p1.y-p2.y, 2) + pow(p1.z-p2.z, 2);
	return (sqrt(distance));
	}
	
void main(){

	//opening file
	FILE * fptr;
	fptr = fopen("./test_data/cell_e5", "r");
	
	//get number of coordintes in the file
	int SIZE = 0;
   	char ch;
  	while ((ch = fgetc(fptr)) != EOF){
       if (ch == '\n' || ch == ' ')
           SIZE++;
   	}
   	
   	//get size of file
   	rewind(fptr);
   	fseek(fptr, 0, SEEK_END);
	long file_size = ftell(fptr);
	fseek(fptr, 0, SEEK_SET);
   	
   	//read contents of file
	char * file_buffer = (char*) malloc(sizeof(char) * file_size);
	size_t result = fread ( file_buffer, 1, file_size, fptr );
	
	//array of points
	Point *points = (Point*)malloc(sizeof(Point) * SIZE);
	char line_buff;
	char char_buff[10];
	double rows[3] = {0.0, 0.0, 0.0};
	int row_id = 0, column = 0, char_id = 0;
	
	double start = omp_get_wtime();
	//parsing file for necessary data	
	omp_set_num_threads(nTHREADS);
	//printf("number of threads: %d\n", omp_get_num_threads());
	
	
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
					points[row_id].x = rows[0];
					points[row_id].y = rows[1];
					points[row_id].z = rows[2];
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
	int n = ((SIZE/3) * ((SIZE-1)/3))/2;
	double distance = 0.0;
	int dist_id = 0;
	double * distances = (double*)malloc(sizeof(double) * n);
	int * possibilities = (int*)malloc(sizeof(int) * 2001);
	int temp = 0;
	
	#pragma omp parallel for
	for ( int m = 0; m<2001; m++){
		possibilities[m] = 0;
		}
	printf("possibilities init complete\n");
	
	double start_distance = omp_get_wtime();
	#pragma omp parallel for
	for ( int i = 0; i<(SIZE/3); i++ ){
		printf("number of threads in for %d \n", omp_get_num_threads());
		for( int j = i+1; j <(SIZE/3); j++){
			printf("%d\n", j);
			distance = odometer(points[i], points[j]);
			#pragma omp critical
			distances[dist_id++] = distance;	
			
			temp = (int)(distance*100);
			possibilities[temp]++;
			}
		}
		
	printf("Distance calculating time: %lf \n", omp_get_wtime()-start_distance);
	
	Frequency * outs = (Frequency *) malloc(sizeof(Frequency) * n);
	int index = 0;
	for ( int i = 0; i < 2001; i++){
		if(possibilities[i] != 0){
			sprintf(outs[index++].length, "%.2lf,%d", i/100.0, possibilities[i]);
			//printf("%s\n", outs[index-1].length);
			}
		}
	
	//free(da);
	free(distances);
	free(outs);
	free(file_buffer);
	free(points);
	
	}
	
	
