# Assigment 3 - HPC 039
## Introduction

The goal of the assignment is to implement multi-threading using OpenMP to achieve task and data parallelism. OpenMP is an API that provides a simple way for parallelism using threads on a shared memory computers. The OpenMP implementation of multi-threading is done by forking from the master thread into set number of slave threads which run concurrently, with the runtime environment allocating them to different processors. OpenMP is particularly well-suited for fine grained parallelism and the programmer needs to perform lesser control of execution flow and memory usage than POSIX threads.

## Problem Description
The problem statement given includes a text file containing a list of co-ordinates.The file must be parsed, and the distances between all points calculated. The output of the program is the frequency of round-off for each calculated distance.The co-ordinates are known to be in the range −10 to 10 and the file input canhave up to 2 32 rows. Another constraint given is that the program may not consume more than 1GiB of memory.
The problem can be divided into the following sub-tasks:
* Reading the file and parsing the input
* Calculating Distance between points
* Increment the corresponding counts
* Memory management
* Parallization

## File Handling and Input
The basic idea is to read the contents of the file into a buffer which is then parsed into corresponding x,y and z co-ordinates, which are of type 'float'. The file is read in blocks, which are 10% of the size of the file. This is to balance the load of the program and to keep the memory usage under 1GiB. To perform the necessary distance calculations, the algorithm chosen require doing two passes of the file in blocks - the first pass maps to the second pass for the distance calculations. Proper indexing and maintaining the file pointer correctly are critical to ensure successful performance of the operations. 

~~~
//First file pass
for ( iblock = 0; iblock < file_size; iblock += block ){

	fseek(fptr, iblock, SEEK_SET);
	im = (iblock + block < file_size)? (iblock + block) : file_size;
	file1 = fread(file_buff1, sizeof(char), im-iblock, fptr);
	parse(file_buff1, im-iblock, Point1);
	
	//Second file pass
	for ( jblock = iblock; jblock < file_size; jblock += block ){
			fseek(fptr, jblock, SEEK_SET);
			jm = (jblock + block < file_size)? (jblock + block) : file_size;
			file2 = fread(file_buff2, sizeof(char), jm-jblock, fptr);
			
			parse(file_buff2, jm-jblock, Point2);

~~~
The parsing is done by iterative read of file buffer contents and converting the string to a float. The white space and next line delimiters are used to identify the x,y and z co-ordinates and saved to a list of struct points. The use of structs provides a ease of use and understanding and it allows contiguous allocation of all necessary variables.
~~~
typedef struct points{
float x;
float y;
float z;
}points;

//allocating arrays of 'points' for first and second passes, block is 10% of file size
points * Point1 = (points*)malloc(sizeof(points)*block);
points * Point2 = (points*)malloc(sizeof(points)*block);

//function for parsing the file buffer
void parse ( char * file, int n, points * Point ){

	char char_buff[10];
	float rows[3] = {0.0, 0.0, 0.0};
	int row_id = 0, column = 0, char_id = 0, imax, jmax;
	char c1 = ' ', c2 = '\n';
		
	for ( int loc = 0; loc < n; loc++){
		if (file[loc] == c1){ //If the delimiter is a white space, then start parsing next co-ordinate
			rows[column] = atof(char_buff);
			column++;
			char_id = 0;
			}
		else{
			if (file[loc] == c2){ //If the delimiter is next-line, then start parsing the next point
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

~~~


## Distance calculation

The distance between two points in the 3D space is given by the standard mathematical formula:
		dist = square root((x 1 − x 2 ) + (y 1 − y 2 ) + (z 1 − z 2 ))
The same is implemented as is in the program. However, due consideration needs to be provided since the ’points’ struct needs to be traversed N ∗(N-1)/2 times. It should also be noted that the sqrt function is the bottleneck in the mathematical operations.
In our implementation, the distances are calculated by running a nested loop where the outer loop iterates from i = [0, im] and the inner loop runs from j = [i + 1, jm], for calculations within the same block and the ones between blocks. The variables im and jm are the iteration limits for the various blocks. The addition, subtraction and the square operations are done as is, since they do not provide too much overhead in the benchmark. The power function was not used, and the square was implemented as product to make small gains in the performance. The square root however was implemented using Intel instrinsics, as small variations from the actual values are permitted according to the problem description.
		
~~~
for ( int ix = 0; ix < imax/24; ix++ ){
		for ( int jx = ix+1; jx < jmax/24; jx++ ){
			temporary = ((Point1[ix].x - Point2[jx].x)*(Point1[ix].x - Point2[jx].x)) 
			+ ((Point1[ix].y-Point2[jx].y)*(Point1[ix].y-Point2[jx].y)) 
			+ ((Point1[ix].z-Point2[jx].z)*(Point1[ix].z-Point2[jx].z));
			
			distance = 100*(_mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(temporary))));
~~~

## Frequency update
The constraint on the file input is that none of the co-ordinates are outside the range [−10, 10] the largest possible distance is between the points (−10, −10, −10) and (10, 10, 10) which is 34.641. Since we are only interested in precision upto 2 decimal places, we can consider an array of ints with index ranging from 0 to 34.64 ∗ 100 = 3464. As we calculate distances, we can multiply the distance by 100, drop the decimal points and increment the corresponding index. 


Continuing from the above code snippet with the nested for loop:
~~~
possibilities[distance]++;
}
}
~~~
## Memory Management
The program involves a large data to be read from the Hard Drive and it involves iterating over a large number. Hence special attention must be given to the memory management and locality. The variables to be careful about are the variables that store the file content and the variables that store the co-ordinates.

The optimal way would be to read chunks of the file to a local buffer and performing operations on the buffer. This avoid unnecessary reads from the hard drive. The file reading is performed the same way in both passes of the file. 

  ~~~
FILE * fptr;  
  
char * file_buff1 = (char *) malloc( block * sizeof(char));
size_t file1;
char * file_buff2 = (char *) malloc( block * sizeof(char));
size_t file2;

fseek(fptr, iblock, SEEK_SET);
im = (iblock + block < file_size)? (iblock + block) : file_size;
file1 = fread(file_buff1, sizeof(char), im-iblock, fptr);
 
  ~~~
  The block size is chosen to be 10% of file size because of the limiting condition. It is known that the file will not be bigger than 2^32, which read into a single array of type 'points' will be 4GiB in size. Since we require two arrays of type 'points' for the algorithm, each array must not consume more than 500 MiB. Adding tolerance to other variables that would consume memory, it was decided to have the limiting case to consume 400MiB for the array of 'points'.
  
  
  The coordinates are defined as a stuct with 3 float variables for x, y and z co-ordinates. The parsed co-ordinates are allocated on the heap (using malloc()). This ensures contiguous allocation of memory allowing faster read and traversal.

  ~~~
typedef struct points{
float x;
float y;
float z;
}points;

points * Point1 = (points*)malloc(sizeof(points)*block);
points * Point2 = (points*)malloc(sizeof(points)*block); 
  ~~~
 The remaining variables are not large enough to make significant impact on the total memory usage. Hence the smallest data type to suit each application was chosen.
  
# Parallization
The parallel processing is implemented with OpenMP. The task to be parallelized is the nested for loop which handles the distance calculation and the frequency update. The OpenMP construct, #pragma omp parallel for, handles the parallel execution of the nested for loop. The option schedule(dynamic, 10) is added to break the for loops in chunks of dynamic size ( since the nested for 4 is a triangular nested for loop ). The option reduction(+:possibilities[3465]) is added to ensure the proper update of the frequency. This also helps avoid running that line in critical mode which would serialize the execution and nullifies the parallel processing.
~~~ 
omp_set_num_threads(nTHREADS);
#pragma omp parallel for schedule(dynamic, 10)
reduction(+:possibilities[:3465])
for ( i = 0; i < ROWS; i++){
	for ( j = i+1; j < ROWS; j++ ) {
		temporary = ((Point1[ix].x - Point2[jx].x)*(Point1[ix].x - Point2[jx].x)) 
			+ ((Point1[ix].y-Point2[jx].y)*(Point1[ix].y-Point2[jx].y)) 
			+ ((Point1[ix].z-Point2[jx].z)*(Point1[ix].z-Point2[jx].z));
			
		distance =
		100*(_mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(temporary))));
		possibilities[distance]++;
		}
	}
~~~ 

# Benchmarking
The target for the assignment was to complete the execution of the program for 1e4 and 1e5 file sizes for 1, 5, 10 and 15 threads. The actual times listed are the average over 10 executions of the program. The target times and the actual times are listed below:



|FileSize    | No of Threads       |    TargetTime(sec)   | Actual Time(sec)        | 
|:-----------|:-------------------:|:--------------------:|:-----------------------:|
|1e4 	     | 1                   |  0.41                | 0.172                   | 
|1e5 	     | 5                   |  8.20                | 3.062                   | 
|1e5 	     | 10                  |  4.10                | 1.613                   | 
|1e5 	     | 20                  |  2.60                | 0.98                    | 

