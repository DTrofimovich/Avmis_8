#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define SIZE 1000000

void initialize_x(float*);
void calculate_function(float*, float*, int);
__gloabal__ void calculate_function_on_cuda(float*, float*, int);
void main()
{
	srand( time(NULL) );
	float *x, *y, *yr;
	float *d_x, *d_y;

	x = malloc( SIZE * sizeof(float) );
	y = malloc( SIZE * sizeof(float) );
	yr = malloc( SIZE * sizeof(float) );

	cudaMalloc( &d_x, SIZE * sizeof(float) );
	cudaMalloc( &d_y, SIZE * sizeof(float) );

	initialize_x(x);

	cudaMemcpy( d_x, x, SIZE * sizeof(float), cudaMemcpyHostToDevice );
	cudaMemcpy( d_y, yr, SIZE * sizeof(float), cudaMemcpyHostToDevice );

	calculate_function( x, y, SIZE );
	calculate_function_on_cuda <<< 1, SIZE >>> ( d_x, d_y, SIZE );

	cudaMemcpy( yr, d_y, SIZE * sizeof(float), cudaMemcpyDeviceToHost );

	free(x);
	free(y);
	free(yr);
	cudaFree(d_x);
	cudaFree(d_y);
	return;
}

void initialize_x(float *x)
{
	int i;
	for(i = 0; i < SIZE; i += 1)
		x[i] = (float) (rand() % 100);
	return;
}


void calculate_function(float *restrict x, float *restrict y, N)
{
	const float a = 2.1;
	const float b = 4.3;
	const float c = 5.5;
	float x_2, x;
	int i;

	for(i = 0; i < N; i++)
	{
		x = x[i];
		x_2 = x * x;
		y[i] = a * x_2 + b * x + c;
	}
	return;
}

__gloabal__ void calculate_function_on_cuda(float* x, float* y, int N)
{
	const float a = 2.1;
	const float b = 4.3;
	const float c = 5.5;
	int i = threadIdx.x;

	if(i < N)
		y[i] = a * x[i] * x[i] + b * x[i] + c;
	return;
}