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
	//short x;
	//short y;
	//short z;
	}points;
	
void dist_inter(points * Point1, int imax, points * Point2, int jmax, int * possibilities){
	float temporary;
	int distance;
	for ( int ix = 0; ix < imax/24; ix++ ){
		for ( int jx = 0; jx < jmax/24; jx++ ){
			temporary = pow((Point1[ix].x - Point2[jx].x),2) + pow((Point1[ix].y-Point2[jx].y),2) + pow((Point1[ix].z-Point2[jx].z),2);
			distance = 100*sqrtf(temporary);
			possibilities[distance]++;
			}
		}
	}
	
void dist_intra(points * Point1, int imax, points * Point2, int jmax, int * possibilities){
	float temporary;
	int distance;
	for ( int ix = 0; ix < imax/24; ix++ ){
		for ( int jx = ix+1; jx < jmax/24; jx++ ){

			temporary = pow((Point1[ix].x - Point2[jx].x),2) + pow((Point1[ix].y-Point2[jx].y),2) + pow((Point1[ix].z-Point2[jx].z),2);
			distance = 100*sqrtf(temporary);
			possibilities[distance]++;
			}
		}
	}
	
void parse ( char * file, int n, points * Point ){

	char char_buff[10];
	float rows[3] = {0.0, 0.0, 0.0};
	int row_id = 0, column = 0, char_id = 0, imax, jmax;
	char c1 = ' ', c2 = '\n';
	//printf("%d\n", n);
		
	for ( int loc = 0; loc < n; loc++){
		if (file[loc] == c1){
			rows[column] = atof(char_buff);
			column++;
			char_id = 0;
			}
		else{
			if (file[loc] == c2){
				rows[column] = atof(char_buff);
				column = 0;
				Point[row_id].x = rows[0];
				Point[row_id].y = rows[1];
				Point[row_id].z = rows[2];
				row_id++;
				char_id = 0;
				}
			else{
				char_buff[char_id] = file[loc];
				char_id++;
				}
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
	fptr = fopen("./cells", "r");
	//fptr = fopen("./test_data/cell_e5", "r");
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
			//printf("entered small file handling\n");
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
						//Point[row_id].x = rows[0]*1000;
						//Point[row_id].y = rows[1]*1000;
						//Point[row_id].z = rows[2]*1000;
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
					//x1 = Point[i].x/1000.0 - Point[j].x/1000.0;
					//y1 = Point[i].y/1000.0 - Point[j].y/1000.0;
					//z1 = Point[i].z/1000.0 - Point[j].z/1000.0;
					//temporary = (pow((Point[i].x/1000.0 - Point[j].x/1000.0),2) + pow((Point[i].y/1000.0-Point[j].y/1000.0),2) + pow((Point[i].z/1000.0-Point[j].z/1000.0),2));
					temporary = (pow((Point[i].x- Point[j].x),2) + pow((Point[i].y-Point[j].y),2) + pow((Point[i].z-Point[j].z),2));
					//temporary = x1*x1 + y1*y1 + z1*z1;
					distance = 100*(_mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(temporary))));
					possibilities[distance]++;
					}
				}	
				
			double end_distance1 = omp_get_wtime();
				
			for ( int i = 0; i < 3465; i++){
				printf("%.2lf %d\n", i/100.0, possibilities[i]);
				}
			
			free(file_buffer);
			free(possibilities);
			free(Point);
			printf("Program time: %lf\n", omp_get_wtime()-start_prog);
			}
		else{
			long block = 0.1*file_size;
	points * Point1 = (points*) malloc(sizeof(points)*block);
	points * Point2 = (points*) malloc(sizeof(points)*block);
	int * possibilities = (int *) calloc(3465, sizeof(int));
	
	char * file_buff1 = (char *) malloc( block * sizeof(char));
	size_t file1;
	char * file_buff2 = (char *) malloc( block * sizeof(char));
	size_t file2;
	
	char char_buff[10];
	float rows[3] = {0.0, 0.0, 0.0};
	float rows1[3] = {0.0,0.0,0.0};
	int row_id = 0, column = 0, char_id = 0, imax, jmax;
	char c1 = ' ', c2 = '\n';
	
	int iblock, jblock, m = 0, im, jm;
	float temporary;
	int distance;	
		
	for ( iblock = 0; iblock < file_size; iblock += block ){

		fseek(fptr, iblock, SEEK_SET);
		im = (iblock + block < file_size)? (iblock + block) : file_size;
		file1 = fread(file_buff1, sizeof(char), im-iblock, fptr);
		parse(file_buff1, im-iblock, Point1);		
		
		fseek(fptr, 0, SEEK_SET);
		
		for ( jblock = iblock; jblock < file_size; jblock += block ){
			jm = (jblock + block < file_size)? (jblock + block) : file_size;
			file2 = fread(file_buff2, sizeof(char), jm-jblock, fptr);
			parse(file_buff2, jm-jblock, Point2);
			if(jblock == iblock){
				dist_intra(Point1, im-iblock, Point2, jm-jblock, possibilities);
				}
			else{
				dist_inter(Point1, im-iblock, Point2, jm-jblock, possibilities);
				}
			}		
		} 
		
	for ( int mx = 0; mx < 3465; mx++ ){
		if(possibilities[mx]!=0){
			printf("%.2f %d\n", mx/100.0, possibilities[mx]);
			}
		}
		
	free(file_buff1);
	free(file_buff2);
	free(Point1);
	free(Point2);
	free(possibilities);
	printf("Program time: %lf \n", omp_get_wtime()-start_prog);
		}
	}
