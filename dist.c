#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>

#define nTHREADS 1

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
	distance = ((p1.x-p2.x) * (p1.x-p2.x)) + ((p1.y-p2.y) * (p1.y-p2.y)) + ((p1.z-p2.z)*(p1.z-p2.z));
	return (sqrt(distance));
	}
	
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
	int n = ((SIZE/3) * ((SIZE/3)-1))/2;
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
	int temp;
	#pragma omp parallel for shared (possibilities)
	for ( int i = 0; i<(SIZE/3); i++ ){
		for( int j = i+1; j <(SIZE/3); j++){
		
			distance = odometer(points[i], points[j]);
			temp = (int)(distance*100);
			possibilities[temp]++;
			
			}
		}
		
	printf("Distance calculating time: %lf \n", omp_get_wtime()-start_distance);

	#pragma omp for
	for ( int i = 0; i < 3465; i++){
		if(possibilities[i] != 0){
			//printf("%.2lf,%d \n", i/100.0, possibilities[i]);
			}
		}
	printf("Total program time: %lf\n", omp_get_wtime()-start_prog);
	
	free(file_buffer);
	free(points);
	}
	
	
