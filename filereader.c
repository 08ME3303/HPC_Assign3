#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

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

void main(int argc, char**argv){

	printf("%d\n", argc);
	FILE * fptr;
	double start_prog = omp_get_wtime();
	fptr = fopen("./test_data/cell_50", "r");
	
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
	long block1 = 0.1*file_size;
	points * Point1 = (points*) malloc(sizeof(points)*block1);
	points * Point2 = (points*) malloc(sizeof(points)*block1);
	int * possibilities = (int *) calloc(3465, sizeof(int));
	printf("%ld, %ld\n", file_size, (block1));
	
	//FILE * fptr2;
	//fptr2 = fopen("./test_data/cells_e3", "r");
   	
   	//read contents of file
	char * file_buffer = (char*) malloc(sizeof(char) * block1);
	char * file_buffer_2 = (char*) malloc(sizeof(char) * block1);
	//size_t result = fread ( file_buffer, 1, file_size, fptr );
	char char_buff[10];
	float rows[3] = {0.0, 0.0, 0.0};
	float rows1[3] = {0.0,0.0,0.0};
	int row_id = 0, column = 0, char_id = 0, imax, jmax;
	char c1 = ' ', c2 = '\n';
	
	for ( int iblock = 0; iblock < file_size; iblock += block1 ){
		imax = (iblock+block1>file_size)?(file_size):(iblock+block1);
		printf("//%d\n", iblock);
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
					printf("%d -------------------%f %f %f\n",iblock, Point1[row_id].x, Point1[row_id].y, Point1[row_id].z);
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
		
		//printf("//%c\n", file_buffer[0]);
		for ( int jblock =0; jblock < file_size; jblock += block1 ){
			jmax = (jblock+block1>file_size)?(file_size):(jblock+block1);
			//printf("====%d\n", jblock);
			fseek(fptr, jblock, SEEK_SET);
			size_t resultj = fread ( file_buffer_2, 1, jmax, fptr);
			//printf("====\t%c\n", file_buffer_2[0]);
			points * Point2 = (points*) calloc(block1, sizeof(points));
			
			column = 0;
			int row_id_1 = 0;
			char_id = 0;

			for ( int l = 0; l < jmax-jblock; l++){
				//printf("second parsing operation --------------------%d, %d, %d\n", l, jblock, jmax);
				if (file_buffer_2[l] == c1){
					rows[column] = atof(char_buff);
					column++;
					char_id = 0;
					}
				else{
					if (file_buffer_2[l] == c2){
						rows[column] = atof(char_buff);
						column = 0;
						printf("%f, %d\n",rows[0], row_id);
						Point2[row_id_1].x = rows[0];
						Point2[row_id_1].y = rows[1];
						Point2[row_id_1].z = rows[2];
						row_id_1++;
						char_id = 0;
						printf("%d %d %f %f %f\n", jblock, l, Point2[row_id-1].x, Point2[row_id-1].y, Point2[row_id-1].z);
						}
					else{
						char_buff[char_id] = file_buffer_2[l];
						char_id++;
						}
					}
				}
			distcalc(row_id, row_id, Point1, Point2, possibilities);
			//free(Point2);
			//printf("Point2 freed\n");
			}
		//printf("exit point2 for\n");
		//free(Point1);
		//printf("Point1 freed\n");
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
