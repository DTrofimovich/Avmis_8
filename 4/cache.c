#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <stdint.h>

#define CACHE_SIZE 32 * 1024
#define OFFSET 1024 * 1024

uint64_t get_ticks()
{
	uint32_t low, high;
	asm( "rdtsc" : "=a" (low), "=d" (high));
	return (uint64_t)low | ((uint64_t)high << 32);
}

struct Element
{
	struct Element *next;
};

void create_array(struct Element *arr, int fragments)
{
	int bank = CACHE_SIZE / sizeof(struct Element) / fragments;
	int offset = OFFSET / sizeof(struct Element);
	int i, j;
	for(i = 0; i < bank - 1; i++)
	{
		for(j = 0; j < fragments - 1; j ++)
		{
			arr[i + j * offset].next = &arr[i + (j + 1) * offset];
		}

		arr[i + (fragments - 1) * offset].next = &arr[i + 1];
	}
	arr[bank - 1 + (fragments - 1) * offset].next = &arr[0];
}

void clear_cache()
{
	int *arr = malloc(CACHE_SIZE);
	int i, tmp;
	int elements = CACHE_SIZE / sizeof(int);
	for(i = 0; i < elements; i++)
		tmp = arr[i];
}
void main()
{
	int i, j, k, tmp;
	clock_t ticks_begin, ticks_end;
	int *mas;
	struct Element *arr, *arr_tmp;
	arr = malloc(200 * 1024 * 1024 + CACHE_SIZE);


	for(i = 1; i <= 20; i++)
	{
		arr_tmp = arr;
		clear_cache();
		create_array(arr_tmp, i);
		ticks_begin = clock();
		for(j = 0; j < 10000000; j++)
		{	for(k = 0; k < 100; k++)
			arr_tmp = arr_tmp->next;
		}
		ticks_end = clock();
		double time = (double)(ticks_end - ticks_begin) / CLOCKS_PER_SEC;
		printf("%f\n", time);
	}
	return;
}