#include <cuda.h>
#include <stdio.h>
#define THREAD_NUM 1024

typedef struct Histogram{
	int start_idx;
	int prefix_num;
} Hist;

__global__ void counting_sort_kernel(int arr[], int histarr[], int size)
{
	size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
	
	if(idx < size)
		atomicAdd(&histarr[arr[idx]], 1);
}

__global__ void output_kernel(int result[], Hist prefix[], int max_val)
{
	size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
		
	__shared__ int count;
	count = prefix[idx].prefix_num;
	__shared__ int start_idx;
	start_idx = prefix[idx].start_idx;

	for(int i = 0; i< count; i++){
		result[start_idx + i] = idx;
	}

}

__host__ void counting_sort(int arr[], int size, int max_val)
{       
	int *d_arr;
	int *d_histogram;
	int *histogram = (int *)malloc(max_val * sizeof(int));

	cudaMalloc(&d_arr, size *sizeof(int));
	cudaMemcpy(d_arr, arr, size * sizeof(int), cudaMemcpyHostToDevice); // initialize unsorted array

	cudaMalloc(&d_histogram, max_val * sizeof(int));
	cudaMemset(d_histogram, 0, max_val * sizeof(int)); // initialize histogram

	int block_num = size / THREAD_NUM;
	if(size % THREAD_NUM)
		block_num = block_num + 1; // Case for remainder;
	counting_sort_kernel <<< block_num, THREAD_NUM >>> (d_arr, d_histogram, size);

	cudaMemcpy(histogram, d_histogram, max_val * sizeof(int), cudaMemcpyDeviceToHost);
	//cudaFree(d_arr);
	cudaFree(d_histogram);

	Hist *histo = (Hist *)malloc(max_val * sizeof(Hist));

	int sum = 0;
	for(int i = 0; i < max_val; i++){
		sum = sum + histogram[i];
		histo[i].prefix_num = histogram[i];
		histo[i].start_idx = sum - histogram[i];
	}

	//int *d_out;
	Hist *d_prefix;

	//cudaMalloc(&d_out, size * sizeof(int));
	cudaMemset(d_arr, 0, size * sizeof(int));

	cudaMalloc(&d_prefix, max_val * sizeof(Hist));
	cudaMemcpy(d_prefix, histo, max_val *sizeof(Hist), cudaMemcpyHostToDevice);

	block_num = max_val / THREAD_NUM;
	if(max_val % THREAD_NUM)
		block_num = block_num + 1;
	output_kernel <<< block_num, THREAD_NUM >>> (d_arr, d_prefix, max_val);
	
	cudaMemcpy(arr, d_arr, size * sizeof(int), cudaMemcpyDeviceToHost);
	cudaFree(d_arr);
	cudaFree(d_prefix);

	free(histogram);
}


