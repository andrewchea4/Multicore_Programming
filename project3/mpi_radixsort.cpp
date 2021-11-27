#include<iostream>
#include<fstream>
#include<string.h>
#include<mpi.h>
#include<omp.h>
#include<unistd.h>

using namespace std;

struct SortData
{
        int count;
        int* next;
        int* prev;
};

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
            cout << "Unable to open file" << endl;
            return 0;
    }

    ret=sscanf(argv[2],"%d", &N);
    if(ret==EOF || N<=0){
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

    size_t max_length = 0;
    
    int *Index  = (int *)malloc(sizeof(int) * N);

    int *Index2 = (int *)malloc(sizeof(int) * N);

    for(i=0; i<N; i++){
        inputfile>>strArr[i];
	Index[i] = i;
        if(max_length < strlen(strArr[i]))
                max_length = strlen(strArr[i]);
    }

    inputfile.close();

    int comm_sz;
    int my_rank;
    
    auto hist_res = new int[16][64];
    auto pair_sum = new int[64*16*2];
    

    auto nodeIndex = new int[N/4]{};
    int hist[66] = {0};
    int prefix_array[64] = {0};
    int start;
    int end;
    int num_flag = 0;
    char** copy_ptr;

    static struct timespec begin2, end2, begin, end3;
    clock_gettime(CLOCK_REALTIME, &begin2);

    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if(N<=0){
	if(my_rank == 0){
        for(i=1; i<N; i++){
            for(j=1; j<N; j++){
                if(strncmp(strArr[j-1], strArr[j],30)>0)
                {
                        strncpy(tmpStr, strArr[j-1], 30);
                        strncpy(strArr[j-1], strArr[j], 30);
                        strncpy(strArr[j], tmpStr, 30);
                }
            }
        }
	cout<<"\nStrings (Names) in Alphabetical order from position " << pos << ": " << endl;
        for(i=pos; i<N && i<(pos+range); i++)
        	cout<< i << ": " << strArr[i]<<endl;
        cout<<endl;
	}
    }
    else{

        //MPI_Init(NULL, NULL);

        //MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
        //MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        int color = (my_rank >= 1)? 0 : 1;
        int key = my_rank;
        int worker_size;
        int worker_rank;
        MPI_Comm worker_comm;
        MPI_Comm_split(MPI_COMM_WORLD, color, key, &worker_comm);
        MPI_Comm_size(worker_comm, &worker_size);
        MPI_Comm_rank(worker_comm, &worker_rank);


        while(max_length != 0){
                for(int i = 0; i< 66; i++)
                        hist[i] = 0;
                for(int i = 0; i<64; i++)
                        prefix_array[i] = 0;

		MPI_Scatter(Index, N/4, MPI_INT, nodeIndex, N/4, MPI_INT, 0, MPI_COMM_WORLD);
		/*
                if(my_rank == 0){

                        //cout << "[MAX LENGTH" << max_length << "]" << endl;
			
			/// TRY 1 ///
                        cout << "PREFIX ARRAY" << endl;
                        for(int i = 0; i < 64; i++)
                                cout << prefix_array[i] << " ";
                        if(num_flag %2 == 0){
                                copy_ptr = copy_str; 
                                //MPI_Bcast(&strArr[0][0], N*30, MPI_CHAR, 0, MPI_COMM_WORLD);
                                //MPI_Bcast(hist, 64, MPI_INT, 0, MPI_COMM_WORLD);
                                //MPI_Bcast(prefix_array, 64, MPI_INT, 0, MPI_COMM_WORLD);
                        }
                        else{
                                copy_ptr = copy2_str;
                                //MPI_Bcast(&strArr[0][0], N*30, MPI_CHAR, 0, MPI_COMM_WORLD);
                                //MPI_Bcast(hist, 64, MPI_INT, 0, MPI_COMM_WORLD);
                                //MPI_Bcast(prefix_array, 64, MPI_INT, 0, MPI_COMM_WORLD);
                        }
                        for(int i = 0; i<N; i++)
                                cout << strArr[i] << endl;
                        cout << "[Master Process]" << endl;

			/// TRY 2 ///
                        int tmp_start;
                        int tmp_end;
                        MPI_Request request;
                        for(int i = 1; i<comm_sz; i++){
                                if(i != comm_sz-1){
                                        tmp_start = (N/(comm_sz -1) + 1) * (i - 1);
                                        tmp_end = (N/(comm_sz -1) + 1) * i - 1; 
                                }
                                else{
                                        tmp_start = (N/(comm_sz -1) + 1) * (i - 1);
                                        tmp_end = N-1;
                                }
                                for(int j = tmp_start; j<=tmp_end; j++){
                                        if(i == comm_sz - 1){
                                                //cout << "Master rank [" << j << "] " <<"strArr[j]" << strArr[j] << endl;
                                                MPI_Isend(strArr[j], 30, MPI_CHAR, i, 10+i, MPI_COMM_WORLD, &request);
                                        }
                                        else{
                                                MPI_Isend(strArr[j], 30, MPI_CHAR, i, 10+i, MPI_COMM_WORLD, &request);
                                        }
                                }
                        }
                }*/

                //for(int i = 0; i<N; i++){
                //        MPI_Bcast(strArr[i], N*30, MPI_CHAR, 0, MPI_COMM_WORLD);
                //}
                //MPI_Barrier(MPI_COMM_WORLD);

                //if(my_rank != 0){ // Make local histogram per process
                        /*if(my_rank == 1){
                                cout << "PREFIX ARRAY" << endl;
                                for(int j = 0; j< 64; j++)
                                        cout << prefix_array[j] << " ";
                                cout << endl; 
                        }*/
                  /*      MPI_Request request_;

                        if(my_rank != comm_sz -1){
                                start = (N/(comm_sz -1) + 1) * (my_rank - 1);
                                end = (N/(comm_sz -1) + 1) * my_rank - 1; 
                        }
                        else{
                                start = (N/(comm_sz -1) + 1) * (my_rank - 1);
                                end = N-1;
                        }
                        for(int j = start; j<=end; j++){
                                if(my_rank == comm_sz -1){
                                //cout << "Before [" << j << "] "<< "strArr[j]: " << strArr[j] << "\t";
                                MPI_Irecv(strArr[j],30, MPI_CHAR, 0, 10+my_rank, MPI_COMM_WORLD,&request_);
                                //cout << "       After [" << j << "]" << "strArr[j]: " << strArr[j] << endl;
                                }
                                else{
                                        MPI_Irecv(strArr[j], 30, MPI_CHAR, 0, 10+my_rank, MPI_COMM_WORLD, &request_);
                                }
                                MPI_Wait(&request_, MPI_STATUS_IGNORE);
                        }
                        MPI_Barrier(worker_comm);
                        //cout << "my rank : " << my_rank << " " << "worker rank :" << worker_rank << " " << "start : " << start << "      end :" << end << endl;
                        int startBuf = start;
                        int endBuf = end;
                        MPI_Request request[2];
                        MPI_Isend(&startBuf, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, &request[0]);
                        MPI_Isend(&endBuf, 1, MPI_INT, 0, 4, MPI_COMM_WORLD, &request[1]);

                        for(int i = 0; i<2; i++){
                                MPI_Wait(&request[i], MPI_STATUS_IGNORE);
                        }
*/
                        #pragma omp parallel num_threads(16) shared(hist)
                        {       
                               #pragma omp for
                                for(int i = 0; i<N/4; i++){
                                        if(strlen(strArr[nodeIndex[i]]) < max_length){
                                                #pragma omp atomic
                                                hist[0]++;
                                        }else{
                                                #pragma omp atomic
                                                hist[strArr[nodeIndex[i]][max_length - 1]-64]++;
                                        }        
                                }
                                #pragma omp barrier
                        }
                        /*if(my_rank == 2){
                                for(int i =0; i<N; i++){
                                        cout << strArr[i] << endl;
                                }
                                cout << "[Rank 2 Process]" << endl;
                        }*/
                       // MPI_Barrier(worker_comm);

                        /*for(int i =1; i<comm_sz; i++){
                                if(my_rank == i){
                                        cout << "[worker rank!!!" << my_rank - 1 << "]" <<"\t";
                                        for(int j = 0; j<64; j++){
                                                cout << hist[j] << " ";
                                        }       
                                        cout << endl;
                                }
                        }
                        cout << endl;*/


                        /*for(int i = start; i<=end; i++){
                                MPI_Request request;
                                if(strlen(strArr[i]) < max_length){
                                        MPI_Isend(&strArr[i], 30, MPI_CHAR, 0, prefix_array[0], MPI_COMM_WORLD, &request);
                                        cout << "my rank" << endl;
                                        prefix_array[0]++;
                                }
                                else{
                                        MPI_Isend(&strArr[i], 30, MPI_CHAR, 0, prefix_array[strArr[i][max_length-1]-64], MPI_COMM_WORLD, &request);
                                        prefix_array[strArr[i][max_length-1]-64]++;
                                }
                        }*/
             //   }
                        bool flag = true;
                        for(int i = 0; i< 59; i++){
                                MPI_Exscan(&hist[i], &prefix_array[i], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
                                if(my_rank == 0){
                                        int recv_temp = 0;
					int temp;
                                        MPI_Status status;
                                        MPI_Recv(&recv_temp, 1, MPI_INT, comm_sz - 1, 1 ,MPI_COMM_WORLD, &status);
                                        hist[i+1] = hist[i+1] + recv_temp;
					if(flag){
						temp = 0;
						flag = false;
					}
					prefix_array[i] = temp;
					temp = recv_temp;
                                        
                                }

                                if(my_rank == comm_sz - 1){
                                        int send_temp = 0;
                                        send_temp = hist[i] + prefix_array[i];
                                        MPI_Send(&send_temp, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
                                }
                                MPI_Barrier(MPI_COMM_WORLD);
                                
                                
                        }

                if(my_rank != 0){
                        MPI_Request prefix_request;
                        MPI_Isend(prefix_array, 64, MPI_INT, 0, 0, MPI_COMM_WORLD, &prefix_request);
                        MPI_Wait(&prefix_request, MPI_STATUS_IGNORE);
                }

                if(my_rank == 0){
                        //cout << "This is master process" << endl;
                        //cout << "worker size : " << comm_sz - 1 << endl;
                        auto recvBuf = new int[comm_sz][2];
                        auto prefixBuf = new int[comm_sz][64];
                        MPI_Request* request = (MPI_Request *)malloc(sizeof(MPI_Request)*(comm_sz - 1)*2);
                        MPI_Request* prefix_req = (MPI_Request *)malloc(sizeof(MPI_Request)*(comm_sz -1));
			/*
                        for(int i = 0; i<comm_sz - 1; i++){
                                MPI_Irecv(&recvBuf[i][0], 1, MPI_INT, i+1, 3, MPI_COMM_WORLD, &request[2*i]);
                                MPI_Irecv(&recvBuf[i][1], 1, MPI_INT, i+1, 4, MPI_COMM_WORLD, &request[2*i+1]);
                        }

                        for(int i = 0; i<(comm_sz - 1)*2; i++){
                                MPI_Wait(&request[i], MPI_STATUS_IGNORE);
                        }
			

                        
			for(int i = 0; i<comm_sz - 1; i++){
                                cout << "rank[" << i << "] :" << "Start : " << recvBuf[i][0] << " " << "End : " << recvBuf[i][1] << endl;
                        }*/

                        for(int i = 0; i<comm_sz - 1; i++){
                                MPI_Irecv(prefixBuf[i+1], 64, MPI_INT, i+1, 0, MPI_COMM_WORLD, &prefix_req[i]);
                        }

                        for(int i = 0; i<comm_sz - 1; i++){
                                MPI_Wait(&prefix_req[i], MPI_STATUS_IGNORE);
                        }

			for(int i = 0; i<59; i++){
				prefixBuf[0][i] = prefix_array[i];

			}
			clock_gettime(CLOCK_REALTIME, &begin);

                        
                        #pragma omp parallel num_threads(30) shared(strArr) shared(prefixBuf)
                        {
                       		#pragma omp for         
                                for(int i = 0; i <comm_sz; i++){
                                        for(int j = 0; j< N/4; j++){

                                                //cout << "[RANK] : " << i  << "   recvBuf[i][0] : " << recvBuf[i][0] << "    recvBuf[i][1] : " << recvBuf[i][1] << endl;
                                                if(strlen(strArr[Index[i*(N/4)+j]])< max_length)
                                                {
                                                        //cout <<  strArr[j] << endl;
                                                        //strcpy(copy_str[prefixBuf[i][0]], strArr[j]);
							Index2[prefixBuf[i][0]] = Index[i*(N/4) + j];
                                                        prefixBuf[i][0]++;
                                                }
                                                else{
                                                        //cout << i << " " << prefixBuf[i][strArr[j][max_length - 1]-64] << " " << strArr[j] << endl;
                                                        //strcpy(copy_str[prefixBuf[i][strArr[j][max_length - 1]-64]], strArr[j]);
                                                        Index2[prefixBuf[i][strArr[Index[i*(N/4)+j]][max_length-1]-64]] = Index[i*(N/4) + j];
							prefixBuf[i][strArr[Index[i*(N/4)+j]][max_length-1]-64]++;
                                                }
                                        }
                                }
                                #pragma omp barrier
                       }
		      clock_gettime(CLOCK_REALTIME, &end3);

                        //for(int i =0; i< N; i++)
                                //cout << copy_str[i] << endl;
                        //cout << "[Intermediate-ENTER]" << endl;
			

                        swap(Index, Index2);

                        //for(int i =0; i< N; i++)
                                //cout << strArr[i] << endl;
                        //cout << "[FINISHED]" << endl;
                        
                        

                        if(max_length == 1){
				#pragma omp parallel num_threads(20) shared(copy_str)
		                {
                		    	#pragma omp for
                  		    	for(int i = 0; i < N; i++){
                        			strcpy(copy_str[i], strArr[Index[i]]);
                    			}
                    			#pragma omp barrier
                		}

                                cout<<"\nStrings (Names) in Alphabetical order from position " << pos << ": " << endl;
                                for(i=pos; i<N && i<(pos+range); i++)
                                        cout<< i << ": " << copy_str[i]<<endl;
                                        cout<<endl;
                                    clock_gettime(CLOCK_REALTIME, &end2);
                                    long time = 1000000000*(end2.tv_sec - begin2.tv_sec) + (end2.tv_nsec - begin2.tv_nsec);
				    long time2 = 1000000000*(end3.tv_sec - begin.tv_sec) + (end3.tv_nsec - begin.tv_nsec);

                                    printf("(Nano): %ld\n", time);
                                    printf("(Micro): %lf\n", (double)time/1000);
                                    printf("(Milli): %lf\n", (double)time/1000000);
				   // printf("time2: %lf\n", (double)time2/1000000);

                        }
                }         
                max_length--; 
                        
        }
                
    }           

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
    	return 0;
}













