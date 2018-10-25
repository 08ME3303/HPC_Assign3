# Assigment 3 - HPC 039

* Earl Fernando Panimayam Fernando
* Anand Hariharan
* Anoop Subramanian
## Introduction

The goal of the assignment is to implement multi-threading using OpenMP to achieve task and data parallelism. OpenMP is an API that provides a simple way for parallelism using threads on a shared memory computers. The OpenMP implementation of multi-threading is done by forking from the master thread into set number of slave threads which run concurrently, with the runtime environment allocating them to different processors. OpenMP is particularly well-suited for fine grained parallelism and the programmer needs to perform lesser control of execution flow and memory usage than POSIX threads.

## Problem Description
The problem statement given includes a text file containing a list of co-ordinates.The file must be parsed, and the distances between all points calculated. Theoutput of the program is the frequency of round-off for each calculated distance.The co-ordinates are known to be in the range −10 to 10 and the file input canhave up to 2 32 rows. Another constraint given is that the program may not consume more than 1GiB of memory.
The problem can be divided into the following sub-tasks:
* Reading the file and parsing the input
* Calculating Distance between points
* Increment the corresponding counts
* Memory management
* Parallization

## File Handling and Input

The file handling was handled by reading the file contents in one go and saving it to a local buffer. The number of operations to be performed for parsing is equal to the file size. Since the file is stored in the hard drive repeated file operations will be quite slow and will be a major bottleneck for hitting the performancebenchmark. Once the contents of the file are read, the file can be closed and the parsing operations can be performed on the heap allocated file buffer variable.

~~~
//opening file
FILE * fptr;
//reading the file to a buffer
char * file_buffer = (char*) malloc(sizeof(char) * file_size);
size_t result = fread ( file_buffer, 1, file_size, fptr );
}
~~~
The parsing is done by iterative read of file buffer contents and converting the string to a float. The white space and next line delimiters are used to identify the x,y and z co-ordinates and saved to a list of struct points. The use of structs provides a ease of use and understanding and it allows contiguous allocation of all necessary variables.
~~~
typedef struct points{
float x;
float y;
float z;
}points;
points * Point = (points*)malloc(sizeof(points)*ROWS);
char c1 = ' ', c2 = '\n';
for ( int loc = 0; loc < file_size; loc++){
if (file_buffer[loc] == c1){
//x and y co-ordinates parsed
rows[column] = atof(char_buff);
column++;
char_id = 0;
}
else{
if (file_buffer[loc] == c2){
//z co-ordinate parsed
rows[column] = atof(char_buff);
column = 0;
Point[row_id].x = rows[0];
Point[row_id].y = rows[1];
Point[row_id].z = rows[2];
row_id++;
char_id = 0;
}
else{
//save the characters till the next delimiter
char_buff[char_id] = file_buffer[loc];
char_id++;
}
}
}
~~~


## Distance calculation

The distance between two points in the 3D space is given by the standard mathematical formula:
		dist = square root((x 1 − x 2 ) + (y 1 − y 2 ) + (z 1 − z 2 ))
The same is implemented as is in the program. However, due consideration needs to be provided since the ’points’ struct needs to be traversed N ∗(N-1)/2 times. It should also be noted that the sqrt function is the bottleneck in the mathematical operations.
In our implementation, the distances are calculated by running a nested loop where the outer loop iterates from i = [0, ROW S] and the inner loop runs from j = [i + 1, ROW S]. The addition, subtraction and the square operations are done as is, since they do not provide too much overhead in the benchmark. The square root however was implemented using Intel instrinsics, as small variations from the actual values are permitted according to the problem description.
		
~~~
for ( i = 0; i < ROWS; i++){
for ( j = i+1; j < ROWS; j++ ) {
temporary = (pow((Point[i].x - Point[j].x),2) +
pow((Point[i].y-Point[j].y),2) +
pow((Point[i].z-Point[j].z),2));
distance =
100*(_mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss
(temporary))));
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
The program involves a large data to be read from the Hard Drive and it involves iterating over a large number. Hence special attention must be given to the memory management and locality. The particular variables to be careful about are the variables that store the file content and the variables that store the co-ordinates.


For smaller files, that can easily fit in the 1 GiB memory limit, the file contents can be read in one operation into a char array (allocated on the heap using malloc()).

  ~~~
  FILE * fptr;
char * file_buffer = (char*) malloc(sizeof(char) * file_size);
size_t result = fread ( file_buffer, 1, file_size, fptr );
 
  ~~~
  
  For files larger than 1 GiB limit, the idea is to perform the task in blocks.
The file reading, parsing and operations are performed in two blocks and the
math operations are performed between the blocks. The block size was chosen
as 10% of the file size (which for the limiting case of file size, 2 32 , results in
block size of approximates 400 MiB).

  ~~~
long block1 = 0.1*file_size;
points * Point1 = (points*)malloc(sizeof(points)*block1);
points * Point2 = (points*)malloc(sizeof(points)*block1);
 
  ~~~
  
  
  The coordinates are defined as a stuct with 3 float variables for x, y and z co-
ordinates. The parsed co-ordinates are allocated on the heap (using malloc()).
This ensures contiguous allocation of memory allowing faster read and traversal.

  ~~~
typedef struct points{
float x;
float y;
float z;
}points;
points * Point = (points*)malloc(sizeof(points)*ROWS);
 
  ~~~
 The remaining variables are not large enough to make significant impact on
the total memory usage. Hence the smallest data type to suit each application
was chosen.
  
# Parallization
The parallel processing is implemented with OpenMP. The task to be parallelized
is the nested for loop which handles the distance calculation and the
frequency update. The OpenMP construct, #pragma omp parallel for, handles
the parallel execution of the nested for loop. The option schedule(dynamic, 10)
is added to break the for loops in chunks of dynamic size ( since the nested for
4 is a triangular nested for loop ). The option reduction(+:possibilities[3465]) is
added to ensure the proper update of the frequency. This also helps avoid running
that line in critical mode which would serialize the execution and nullifies
the parallel processing.
~~~ 
omp_set_num_threads(nTHREADS);
#pragma omp parallel for schedule(dynamic, 10)
reduction(+:possibilities[:3465])
for ( i = 0; i < ROWS; i++){
for ( j = i+1; j < ROWS; j++ ) {
temporary = (pow((Point[i].x - Point[j].x),2) +
pow((Point[i].y-Point[j].y),2) +
pow((Point[i].z-Point[j].z),2));
distance =
100*(_mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(temporary))));
possibilities[distance]++;
}
}
~~~ 

# Benchmarking
The target for the assignment was to complete the execution of the program for
1e4 and 1e5 file sizes for 1, 5, 10 and 15 threads. The actual times listed are
the average over 10 executions of the program. The target times and the actual
times are listed below:


|FileSize    | No of Threads       |    TargetTime(sec)   | Actual Time(sec)        | 
|:-----------|:-------------------:|:--------------------:|:-----------------------:|
|1e4 	     | 1                   |  0.41                | 0.27                    | 
|1e5 	     | 5                   |  8.20                | 4.59                    | 
|1e5 	     | 10                  |  4.10                | 2.34                    | 
|1e5 	     | 20                  |  2.60                | 1.49                    | 


 
