#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <malloc.h>
#include <omp.h>


float** allocation(int n, int m);
float** initializematrix(int n, int m);
void outputmatrix(float** mas, int n, int m, char*);
float** vectorize_multiplication(float**, float**, int, int, int);
float** parallel_vectorize_multiplication(float**, float**, int, int, int);
float** asm_multiplication(float**, float**, int, int, int);
float** parallel_asm_multiplication(float**, float**, int, int, int);
float** transp(float**, int, int);


void main()
{
	long time_begin, time_end;
	int n1, m1, n2, m2;
	float **mas1, **mas2, **mas2_with_transposition, **mas3;

	srand(time(0));
	omp_set_dynamic(0);
	omp_set_num_threads(omp_get_max_threads());
	n1 = 2000;
	m1 = 500;
	n2 = m1;
	m2 = 2000;

	mas1 = initializematrix(n1, m1);
	mas2 = initializematrix(n2, m2);
	mas2_with_transposition = transp(mas2, m2, n2);


	time_begin = clock();
	mas3 = vectorize_multiplication(mas1, mas2, n1, m1, m2);
	time_end = clock();
	printf("auto-vectorize: %lf sec\n", (double)(time_end - time_begin)/CLOCKS_PER_SEC);
	outputmatrix(mas3, n1, m2, "auto-vectorize");

	time_begin = clock();
	mas3 = asm_multiplication(mas1, mas2_with_transposition, n1, m1, m2);
	time_end = clock();
	printf("asm-vectorize: %lf sec\n", (double)(time_end - time_begin)/CLOCKS_PER_SEC);
	outputmatrix(mas3, n1, m2, "asm");

	time_begin = clock();
	mas3 = vectorize_multiplication(mas1, mas2, n1, m1, m2);
	time_end = clock();
	printf("parallel auto-vectorize: %lf sec\n", (double)(time_end - time_begin)/CLOCKS_PER_SEC);
	outputmatrix(mas3, n1, m2, "auto-vectorize");

	time_begin = clock();
	mas3 = asm_multiplication(mas1, mas2_with_transposition, n1, m1, m2);
	time_end = clock();
	printf("parallel asm-vectorize: %lf sec\n", (double)(time_end - time_begin)/CLOCKS_PER_SEC);
	outputmatrix(mas3, n1, m2, "asm");
	return;
}

float** allocation(int n, int m)
{
	float** mas;
	int i;
	mas = (float**)memalign(16, n * sizeof(float*));
	for (i = 0; i < n; i++)
		mas[i] = (float*)memalign(16, m * sizeof(float));
	return mas;	
}
float** initializematrix(int n, int m)
{
	float** mas;
	int i, j;
	mas = allocation(n, m);
	for (i = 0; i < n; i++)
		for (j = 0; j < m; j++)
			mas[i][j] = (float)(rand() % 10);
	return mas;
}

float** transp(float** mas, int n, int m)
{
	float **mas3 = allocation(n, m);
	int i, j;
	for( i = 0; i< m; i++)
		for( j = 0; j< n; j++)
			mas3[j][i] = mas[i][j];
	return mas3;
}

void outputmatrix(float** mas, int n, int m, char *filename)
{
	FILE *f = fopen(filename,"w");
	int i, j;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < m; j++)
			fprintf(f, "%.1f ", mas[i][j]);
		fprintf(f, "\n ");
	}
	fclose(f);
	return;
}

float** vectorize_multiplication(float** mas1, float** mas2, int n1, int m1, int m2)
{
	int i, j, k;
	float f, *_mas3, *_mas2;
	float** mas3 = allocation(n1, m2);
	for (i = 0; i < n1; i++)
	{
		_mas3 = mas3[i];
		for (k = 0; k < m1; k++)
		{
			f = mas1[i][k];
			_mas2 = mas2[k];
			for (j = 0; j < m2; j++)
				_mas3[j] += f * _mas2[j];
		}
		
	}
	return mas3;
}

float** parallel_vectorize_multiplication(float** mas1, float** mas2, int n1, int m1, int m2)
{
	int i, j, k;
	float f, *_mas3, *_mas2;
	float** mas3 = allocation(n1, m2);
	#pragma omp parallel shared(mas3, mas2, mas1) private(i, j, k)
	{
		#pragma omp for
		for (i = 0; i < n1; i++)
		{
			_mas3 = mas3[i];
			for (k = 0; k < m1; k++)
			{
				f = mas1[i][k];
				_mas2 = mas2[k];
				for (j = 0; j < m2; j++)
					_mas3[j] += f * _mas2[j];
			}
			
		}
	}
	return mas3;
}
float** asm_multiplication(float** mas1, float** mas2, int n1, int m1, int m2)
{
	int i, j, k;
	float** mas3 = allocation(n1, m2);
	float *ms1, *ms2, result;
	float *end_loop;
	for (i = 0; i < n1; i++)
		for (j = 0; j < m2; j++)
		{
			result = 0;
			mas3[i][j] = 0;
			ms1 = mas1[i];
			ms2 = mas2[j];
			end_loop = &mas1[i][m1];

			__asm{
				mov rax, ms1
				mov rbx, ms2
				mov rdx, end_loop
				xorps xmm2, xmm2
				loop:
					movaps xmm0, [rax]
					movaps xmm1, [rbx]

					mulps xmm0, xmm1

					addps xmm2, xmm0

					add rax, 16
					add rbx, 16
					cmp rdx, rax
					jne loop

				haddps xmm2, xmm2
				haddps xmm2, xmm2

				movss result, xmm2
			}

			mas3[i][j] = result;
		}
	return mas3;	
}

float** parallel_asm_multiplication(float** mas1, float** mas2, int n1, int m1, int m2)
{
	int i, j, k;
	float** mas3 = allocation(n1, m2);
	float *ms1, *ms2, result;
	float *end_loop;
	#pragma omp parallel shared(mas3, mas2, mas1) private(i, j)
	{
		#pragma omp for
		for (i = 0; i < n1; i++)
			for (j = 0; j < m2; j++)
			{
				result = 0;
				mas3[i][j] = 0;
				ms1 = mas1[i];
				ms2 = mas2[j];
				end_loop = &mas1[i][m1];

				__asm{
					mov rax, ms1
					mov rbx, ms2
					mov rdx, end_loop
					xorps xmm2, xmm2
					loop:
						movaps xmm0, [rax]
						movaps xmm1, [rbx]

						mulps xmm0, xmm1

						addps xmm2, xmm0

						add rax, 16
						add rbx, 16
						cmp rdx, rax
						jne loop

					haddps xmm2, xmm2
					haddps xmm2, xmm2

					movss result, xmm2
				}

				mas3[i][j] = result;
			}
	}
	return mas3;	
}