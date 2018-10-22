#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include <xmmintrin.h>

typedef struct points{
	float x;
	float y;
	float z;
	}points;
	
void distcalc( int row_1, int row_2, points * Point1, points * Point2, int * possibilities){
	float temporary;
	int distance;
	for ( int ix = 0; ix < row_1; ix++ ){
		for ( int jx = 0; jx < row_2; jx++ ){
			temporary = (pow((Point1[ix].x - Point2[jx].x),2) + pow((Point1[ix].y-Point2[jx].y),2) + pow((Point1[ix].z-Point2[jx].z),2));
			distance = 100*sqrt(temporary);
			possibilities[distance]++;
			}
		}
	}
	
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
	//fptr = fopen("./cells", "r");
	fptr = fopen("./test_data/cell_e4", "r");
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
	
	char capacity = (file_size>1e7)?('l'):('s');
	
	if ( capacity == 's'){
			//read contents of file
			char * file_buffer = (char*) malloc(sizeof(char) * file_size);
			size_t result = fread ( file_buffer, 1, file_size, fptr );
			
			//array of points
			points * Point = (points*)malloc(sizeof(points)*ROWS);
			
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
						Point[row_id].x = rows[0];
						Point[row_id].y = rows[1];
						Point[row_id].z = rows[2];
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
					temporary = (pow((Point[i].x - Point[j].x),2) + pow((Point[i].y-Point[j].y),2) + pow((Point[i].z-Point[j].z),2));
					distance = 100*(_mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(temporary))));
					possibilities[distance]++;
					}
				}	
				
			double end_distance1 = omp_get_wtime();
			//num_ops = 0;
				
			for ( int i = 0; i < 3465; i++){
				printf("%.2lf %d\n", i/100.0, possibilities[i]);
				}
				
			// Debugging and benchmarking outputs
			//printf("%lld\n", num_ops);
			//printf("Time for distance calculations: %lf\n", end_distance1-start_distance1);
			//printf("Total program time: %lf\n", omp_get_wtime()-start_prog);
			
			free(file_buffer);
			free(Point);
			}
		else{
			long block1 = 0.1*file_size;
			points * Point1 = (points*) malloc(sizeof(points)*block1);
			points * Point2 = (points*) malloc(sizeof(points)*block1);
			int * possibilities = (int *) calloc(3465, sizeof(int));
		   	
		   	//read contents of file
			char * file_buffer = (char*) malloc(sizeof(char) * block1);
			char * file_buffer_2 = (char*) malloc(sizeof(char) * block1);

			char char_buff[10];
			float rows[3] = {0.0, 0.0, 0.0};
			float rows1[3] = {0.0,0.0,0.0};
			int row_id = 0, column = 0, char_id = 0, imax, jmax;
			char c1 = ' ', c2 = '\n';
			
			for ( int iblock = 0; iblock < file_size; iblock += block1 ){
				imax = (iblock+block1>file_size)?(file_size):(iblock+block1);
				fseek(fptr, iblock, SEEK_SET);
				size_t result = fread ( file_buffer, 1, imax, fptr );
				column = 0;
				row_id = 0;
				char_id = 0;
				points * Point1 = (points*) calloc(block1, sizeof(points));
				
				for ( int loc = 0; loc < imax-iblock; loc++){
					if (file_buffer[loc] == c1){
						rows[column] = atof(char_buff);
						column++;
						char_id = 0;
						}
					else{
						if (file_buffer[loc] == c2){
							rows[column] = atof(char_buff);
							column = 0;
							Point1[row_id].x = rows[0];
							Point1[row_id].y = rows[1];
							Point1[row_id].z = rows[2];
							row_id++;
							char_id = 0;
							}
						else{
							char_buff[char_id] = file_buffer[loc];
							char_id++;
							}
						}
					}
				fseek(fptr, 0, SEEK_SET);
				
				for ( int jblock =0; jblock < file_size; jblock += block1 ){
					jmax = (jblock+block1>file_size)?(file_size):(jblock+block1);
					fseek(fptr, jblock, SEEK_SET);
					size_t resultj = fread ( file_buffer_2, 1, jmax, fptr);
					points * Point2 = (points*) calloc(block1, sizeof(points));
					
					column = 0;
					int row_id_1 = 0;
					char_id = 0;

					for ( int l = 0; l < jmax-jblock; l++){
						if (file_buffer_2[l] == c1){
							rows[column] = atof(char_buff);
							column++;
							char_id = 0;
							}
						else{
							if (file_buffer_2[l] == c2){
								rows[column] = atof(char_buff);
								column = 0;
								Point2[row_id_1].x = rows[0];
								Point2[row_id_1].y = rows[1];
								Point2[row_id_1].z = rows[2];
								row_id_1++;
								char_id = 0;
								}
							else{
								char_buff[char_id] = file_buffer_2[l];
								char_id++;
								}
							}
						}
					distcalc(row_id, row_id, Point1, Point2, possibilities);
					}
				}
				
			fclose(fptr);
			
			for ( int mx = 0; mx < 3465; mx++ ){
				if ( possibilities[mx]!=0){
					printf("%d\n", possibilities[mx]);
					}
				}
			
			printf("File closed\n");
			
			free(Point1);
			free(Point2);
			free(possibilities);
		}
	}
