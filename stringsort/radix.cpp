#include <iostream>
#include <fstream>
#include <string.h>
#include <omp.h>
#include <math.h>

using namespace std;

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

    	auto strArr = new char[N][30];
	auto hist_res = new int[16][64];
	auto pair_sum = new int[64*16*2];
	size_t max_length = 0;

    	for(i=0; i<N; i++){
        	inputfile>>strArr[i];
		if(max_length < strlen(strArr[i]))
				max_length = strlen(strArr[i]);
	}

    	inputfile.close();

	int hist[64] = {0};
	pair_sum[64*16*2] = {0};
	int index = 0;
	max_length = 1;

#pragma omp parallel num_threads(16) shared(index) shared(hist_res) firstprivate(hist)
{
	#pragma omp for 
	for(i = 0; i<N; i++){
		if(strlen(strArr[i]) < max_length)
			hist[0]++;
		else
			hist[strArr[i][max_length-1]-64]++;
	}
	#pragma omp critical
	{
		for(int j = 0; j<64; j++)
			hist_res[index][j] = hist[j];
		index++;
	}
#pragma omp barrier
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

	//for(int j = 0; j<8; j++)
	//	pair_sum[(16 + 1*(omp_get_thread_num())+j*2)/2] = pair_sum[16 + 1*(omp_get_thread_num())+j*2] + pair_sum[16 + 1*(omp_get_thread_num())+j*2+1];

#pragma omp barrier
}

#pragma omp parallel num_threads(8) shared(pair_sum)
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
}
	
	pair_sum[1] = pair_sum[2] + pair_sum[3];
	cout << pair_sum[2] << " " <<  pair_sum[3] << endl;


	for(int j = 0; j<16; j++){
		for(int k = 0; k<64; k++)
			cout << hist_res[j][k];
		cout<<endl;
	}
	cout<<endl;

    	cout<<"\nStrings (Names) in Alphabetical order from position " << pos << ": " << endl;
    	//for(i=pos; i<N && i<(pos+range); i++)
        	//cout<< i << ": " << strArr[i]<<endl;
    	//cout<<endl;

    	delete[] strArr;

    	return 0;
}

	
