#include <iostream>
#include <fstream>
#include <string.h>
#include <omp.h>
#include <math.h>

using namespace std;



void prefix_sum(int idx, int left, bool f_flag, int* pair_sum, int* prefix_array)
{
	if(f_flag){
	{
		{
			int lf_input = 0;
			int rt_input = pair_sum[2] + lf_input;
			prefix_sum(2, lf_input, false, pair_sum, prefix_array);
			prefix_sum(3, rt_input, false, pair_sum, prefix_array);
		}
	}
}
	else{
		int lf_input = left;
		int rt_input = pair_sum[2*idx] + lf_input;

		if(2*idx >= 64*16){
			prefix_array[2*idx - 64*16] = lf_input + pair_sum[2*idx];
			prefix_array[2*idx + 1 - 64*16] = rt_input + pair_sum[2*idx+1];
			return;
		}
		{
			{
				prefix_sum(2*idx, lf_input, false, pair_sum, prefix_array);
				prefix_sum(2*idx + 1, rt_input, false, pair_sum, prefix_array);
	
			}
		}
	}
	
}


int main(int argc, char* argv[])
{
	char tmpStr[30];
	int i, j, N, pos, range, ret;

	if(argc<5){
		cout << "Usage: " << argv[0] << " filename number_of_strings pos range" << endl;
		return 0;
	}

	ifstream inputfile(argv[1]);
	if(!inputfile.is_open()){
		cout<< "Unable to open file" << endl;
		return 0;
	}

	ret=sscanf(argv[2], "%d", &N);
	if(ret==EOF|| N<=0){
		cout << "Invalid number" << endl;
		return 0;
	}

	ret=sscanf(argv[3],"%d", &pos);
	if(ret==EOF || pos<0 || pos>=N){
		cout << "Invalid position" << endl;
		return 0;
	}
	ret=sscanf(argv[4],"%d", &range);
	if(ret==EOF || range<0 || (pos+range)>=N){
		cout << "Invalid range" << endl;
            	return 0;
    	}

	char **strArr;
	strArr = (char **)malloc(sizeof(char *) * N);
	for(int j = 0; j< N; j++)
		strArr[j] = (char *)malloc(sizeof(char) * 30);
	
	char **copy_str;
	copy_str = (char **)malloc(sizeof(char *) * N);
	for(int j = 0; j<N; j++)
		copy_str[j] = (char  *)malloc(sizeof(char) * 30);

	char **copy2_str;
	copy2_str = (char **)malloc(sizeof(char *) *N);
	for(int j = 0; j<N; j++)
		copy2_str[j] = (char *)malloc(sizeof(char) * 30);

	auto hist_res = new int[16][64];
	auto pair_sum = new int[64*16*2];
	auto prefix_array = new int[64*16];
	size_t max_length = 0;

    	for(i=0; i<N; i++){
        	inputfile>>strArr[i];
		if(max_length < strlen(strArr[i]))
				max_length = strlen(strArr[i]);
	}


    	inputfile.close();
	
	static struct timespec begin, end;
	clock_gettime(CLOCK_REALTIME, &begin);

	int hist[66] = {0};
	auto count_arr = new int[16][3];
	pair_sum[64*16*2] = {0};
	bool flag = false;
	int num_flag = 0;
	int index = 0;
	int count = 0;
	int temp_idx;
	int temp_count;
	int temp_thd;
	int temp_hist[64];
	char** copy_ptr;
	
	if(N<=1600){
		for(i = 1; i<N; i++)
		{
			for(j = 1; j<N; j++)
			{
				if(strncmp(strArr[j-1], strArr[j], 30)>0)
				{
					strncpy(tmpStr, strArr[j-1], 30);
					strncpy(strArr[j-1], strArr[j], 30);
					strncpy(strArr[j], tmpStr, 30);
				}
			}
		}
	}
	else{
	while(max_length != 0){
		if(num_flag % 2 == 0)
			copy_ptr = copy_str;
		else
			copy_ptr = copy2_str;
	#pragma omp parallel num_threads(16) shared(hist_res) shared(count_arr) firstprivate(hist) firstprivate(flag) firstprivate(count) firstprivate(index)
	{
		#pragma omp for 
		for(i = 0; i<N; i++){
			if(!flag){ // save thread id
				index = i;
				flag = true;
			}

			count ++;
			if(strlen(strArr[i]) < max_length)
				hist[0]++;
			else{
				hist[strArr[i][max_length-1]-64]++;
			}
		}
		#pragma omp critical
		{
			for(int j = 0; j<64; j++)
				hist_res[omp_get_thread_num()][j] = hist[j];
			count_arr[omp_get_thread_num()][0] = index;
			count_arr[omp_get_thread_num()][1] = count;
			count_arr[omp_get_thread_num()][2] = omp_get_thread_num();

		}		
	#pragma omp barrier

	}	
		for(int j =0 ; j<15; j++){
			for(int k = 0; k<15-j;k++){
				if(count_arr[k][0] > count_arr[k+1][0])
				{
					temp_idx = count_arr[k][0];
					temp_count = count_arr[k][1];
					temp_thd = count_arr[k][2];

					count_arr[k][0] = count_arr[k+1][0];
					count_arr[k][1] = count_arr[k+1][1];
					count_arr[k][2] = count_arr[k+1][2];

					count_arr[k+1][0] = temp_idx;
					count_arr[k+1][1] = temp_count;
					count_arr[k+1][2] = temp_thd;

					for(int p = 0; p<64; p++){
						temp_hist[p] = hist_res[k][p];
						hist_res[k][p] = hist_res[k+1][p];
						hist_res[k+1][p] = temp_hist[p];
					}

				}
			}
		}
	#pragma omp parallel num_threads(16) shared(pair_sum) firstprivate(hist_res)
	{
		for(int j = 0; j < 8; j++){
			pair_sum[64*16 + 64*(omp_get_thread_num())+j*2] = hist_res[j*2][omp_get_thread_num()*4];
			pair_sum[64*16 + 64*(omp_get_thread_num())+j*2+1] = hist_res[j*2+1][omp_get_thread_num()*4];
			pair_sum[(64*16 + 64*(omp_get_thread_num())+j*2)/2] = hist_res[j*2][omp_get_thread_num()*4] + hist_res[j*2+1][omp_get_thread_num()*4];

			pair_sum[64*16 + 64*(omp_get_thread_num())+16+j*2] = hist_res[j*2][omp_get_thread_num()*4+1];
			pair_sum[64*16 + 64*(omp_get_thread_num())+16+j*2+1] = hist_res[j*2+1][omp_get_thread_num()*4+1];
			pair_sum[(64*16 + 64*(omp_get_thread_num())+16+j*2)/2] = hist_res[j*2][omp_get_thread_num()*4+1] + hist_res[j*2+1][omp_get_thread_num()*4+1];
		
			pair_sum[64*16 + 64*(omp_get_thread_num())+32+j*2] = hist_res[j*2][omp_get_thread_num()*4+2];
	                pair_sum[64*16 + 64*(omp_get_thread_num())+32+j*2+1] = hist_res[j*2+1][omp_get_thread_num()*4+2];
 	               pair_sum[(64*16 + 64*(omp_get_thread_num())+32+j*2)/2] = hist_res[j*2][omp_get_thread_num()*4+2] + hist_res[j*2+1][omp_get_thread_num()*4+2];

			pair_sum[64*16 + 64*(omp_get_thread_num())+48+j*2] = hist_res[j*2][omp_get_thread_num()*4+3];
        	        pair_sum[64*16 + 64*(omp_get_thread_num())+48+j*2+1] = hist_res[j*2+1][omp_get_thread_num()*4+3];
                	pair_sum[(64*16 + 64*(omp_get_thread_num())+48+j*2)/2] = hist_res[j*2][omp_get_thread_num()*4+3] + hist_res[j*2+1][omp_get_thread_num()*4+3];

		}

		for(int j = 0; j< 8; j++){
			pair_sum[(32*16 + 32*(omp_get_thread_num())+j*2)/2] = pair_sum[32*16 + 32*(omp_get_thread_num())+j*2] + pair_sum[32*16 + 32*(omp_get_thread_num())+j*2+1];
			pair_sum[(32*16 + 32*(omp_get_thread_num())+16+j*2)/2] = pair_sum[32*16 + 32*(omp_get_thread_num())+16+j*2] + pair_sum[32*16 + 32*(omp_get_thread_num())+16+j*2+1];
		}

		for(int j = 0; j< 8; j++)
			pair_sum[(16*16 + 16*(omp_get_thread_num())+j*2)/2] = pair_sum[16*16 + 16*(omp_get_thread_num())+j*2] + pair_sum[16*16 + 16*(omp_get_thread_num())+j*2+1];

		for(int j = 0; j<4; j++)
			pair_sum[(8*16 + 8*(omp_get_thread_num())+j*2)/2] = pair_sum[8*16 + 8*(omp_get_thread_num())+j*2] + pair_sum[8*16 + 8*(omp_get_thread_num())+j*2+1];

		for(int j =0; j<2; j++)
			pair_sum[(4*16 + 4*(omp_get_thread_num())+j*2)/2] = pair_sum[4*16 + 4*(omp_get_thread_num())+j*2] + pair_sum[4*16 + 4*(omp_get_thread_num())+j*2+1];

		for(int j = 0; j<1; j++)
			pair_sum[(2*16 + 2*(omp_get_thread_num())+j*2)/2] = pair_sum[2*16 + 2*(omp_get_thread_num())+j*2] + pair_sum[2*16 + 2*(omp_get_thread_num())+j*2+1];

	#pragma omp barrier
	}

	 	pair_sum[8] = pair_sum[16] + pair_sum[17];
                pair_sum[9] = pair_sum[18] + pair_sum[19];
                pair_sum[10] = pair_sum[20] + pair_sum[21];
                pair_sum[11] = pair_sum[22] + pair_sum[23];
                pair_sum[12] = pair_sum[24] + pair_sum[25];
                pair_sum[13] = pair_sum[26] + pair_sum[27];
                pair_sum[14] = pair_sum[28] + pair_sum[29];
                pair_sum[15] = pair_sum[30] + pair_sum[31];
                pair_sum[4] = pair_sum[8] + pair_sum[9];
                pair_sum[5] = pair_sum[10] + pair_sum[11];
                pair_sum[6] = pair_sum[12] + pair_sum[13];
                pair_sum[7] = pair_sum[14] + pair_sum[15];
                pair_sum[2] = pair_sum[4] + pair_sum[5];
                pair_sum[3] = pair_sum[6] + pair_sum[7];
                pair_sum[1] = pair_sum[2] + pair_sum[3];
	
	/*#pragma omp parallel num_threads(8) shared(pair_sum)
	{
		pair_sum[(16 + omp_get_thread_num()*2)/2] = pair_sum[16 + omp_get_thread_num()*2] + pair_sum[16 + omp_get_thread_num()*2 + 1];

	#pragma omp barrier
	}

	#pragma omp parallel num_threads(4) shared(pair_sum)
	{
		pair_sum[(8 + omp_get_thread_num()*2)/2] = pair_sum[8 + omp_get_thread_num()*2] + pair_sum[8 + omp_get_thread_num()*2 + 1];

	#pragma omp barrier
	}

	#pragma omp parallel num_threads(2) shared(pair_sum)
	{
		pair_sum[(4+ omp_get_thread_num()*2)/2] = pair_sum[4+ omp_get_thread_num()*2] + pair_sum[4 + omp_get_thread_num()*2 + 1];

	#pragma omp barrier
	}*/


	prefix_sum(0, 0, true, pair_sum, prefix_array);

		auto index_arr = new int[16][64];

		for (int j =64*16; j > 0; j--)
			prefix_array[j] = prefix_array[j-1]; 
		prefix_array[0] = 0;

	#pragma omp parallel num_threads(16) shared (prefix_array) shared(index_arr)
	{
		for(int j =0; j<64; j++){
			index_arr[omp_get_thread_num()][j] = prefix_array[omp_get_thread_num() + omp_get_num_threads() * j];
		}
	#pragma omp barrier
	}
	#pragma omp parallel num_threads(16) shared(strArr) shared(index_arr) shared(count_arr) shared(count)
	{
		int start = count_arr[omp_get_thread_num()][0];
		int end = count_arr[omp_get_thread_num()][1];
		int thd = count_arr[omp_get_thread_num()][2];


		for(int j =start; j<start+end; j++){
			if(strlen(strArr[j]) < max_length)
			{
				strcpy(copy_ptr[index_arr[thd][0]], strArr[j]);
				index_arr[thd][0]++;
			}
			else{
				strcpy(copy_ptr[index_arr[thd][strArr[j][max_length-1] - 64]], strArr[j]);
				index_arr[thd][strArr[j][max_length-1] - 64]++;
			}
		}
	#pragma omp barrier
	}
		max_length--;
		strArr = copy_ptr;
		num_flag++;

	
}
}
	clock_gettime(CLOCK_REALTIME, &end);
    	cout<<"\nStrings (Names) in Alphabetical order from position " << pos << ": " << endl;
    	for(i=pos; i<N && i<(pos+range); i++)
        	cout<< i << ": " << strArr[i]<<endl;
    	cout<<endl;

    	delete[] strArr;
	long time = 1000000000*(end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec);
	printf("(Nano): %ld\n", time);
        printf("(Micro): %lf\n", (double)time/1000);
        printf("(Milli): %lf\n", (double)time/1000000);



    	return 0;
}

	
