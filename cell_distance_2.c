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
	
int nTHREADS;
	
void dist_inter(points * Point1, int imax, points * Point2, int jmax, int * possibilities){
	float temporary;
	int distance;
	omp_set_num_threads(nTHREADS);
	#pragma omp parallel for schedule(dynamic, 10) reduction(+:possibilities[:3465])
	for ( int ix = 0; ix < imax/24; ix++ ){
		for ( int jx = 0; jx < jmax/24; jx++ ){
			temporary = pow((Point1[ix].x - Point2[jx].x),2) + pow((Point1[ix].y-Point2[jx].y),2) + pow((Point1[ix].z-Point2[jx].z),2);
			distance = 100*(_mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(temporary))));
			possibilities[distance]++;
			}
		}
	}
	
void dist_intra(points * Point1, int imax, points * Point2, int jmax, int * possibilities){
	float temporary;
	int distance;
	omp_set_num_threads(nTHREADS);
	#pragma omp parallel for schedule(dynamic, 10) reduction(+:possibilities[:3465])
	for ( int ix = 0; ix < imax/24; ix++ ){
		for ( int jx = ix+1; jx < jmax/24; jx++ ){

			temporary = pow((Point1[ix].x - Point2[jx].x),2) + pow((Point1[ix].y-Point2[jx].y),2) + pow((Point1[ix].z-Point2[jx].z),2);
			distance = 100*(_mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(temporary))));
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
	
	if (argc == 2){
		nTHREADS=strtol((strtok(argv[1], "-t")),NULL,10); 
		}
	else{
		printf("Incorrect number of arguments\n");
		return;
		}
		
	double start = omp_get_wtime();
	
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
			fseek(fptr, jblock, SEEK_SET);
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
	int num_ops = 0;
		
	for ( int mx = 0; mx < 3465; mx++ ){
		if(possibilities[mx]!=0){
			num_ops += possibilities[mx];
			printf("%.2f %d\n", mx/100.0, possibilities[mx]);
			}
		}
		
	//printf("Total time: %lf\n", omp_get_wtime()-start_prog);
	free(file_buff1);
	free(file_buff2);
	free(Point1);
	free(Point2);
	free(possibilities);
	}
	
