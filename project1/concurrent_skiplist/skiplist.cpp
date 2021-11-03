/*
 * main.cpp
 *
 * Serial version
 *
 * Compile with -O2
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "skiplist.h"

#include <queue>
#include <pthread.h>

using namespace std;

bool flag = false;
bool print_flag = false;
queue<pair<char, long>> q;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_barrier_t b;

void* worker (void* argv){
    pair<char, long> n;
    skiplist<int, int> *list = (skiplist<int, int> *)argv;
    while(true){
	if(print_flag){
	    pthread_barrier_wait(&b);
	    pthread_barrier_wait(&b);
	}
	pthread_mutex_lock(&m);
        if(q.empty() && !flag){
	    if(!print_flag)
  	    	pthread_cond_wait(&c,&m);
	    pthread_mutex_unlock(&m);
	    continue;
        }
	else if(q.empty() && flag){
	    pthread_mutex_unlock(&m);
	    break;
	}
	else if(!q.empty()){
 	    n = q.front();
            q.pop();
	    pthread_mutex_unlock(&m);
	}
       
        if (n.first == 'i') {            // insert
            list->insert(n.second,n.second);
        }else if (n.first == 'q') {      // qeury
            if(list->find(n.second)!=n.second)
                cout << "ERROR: Not Found: " << n.second << endl;
        } else if (n.first == 'w') {     // wait
            usleep(n.second);
        } else {
            printf("ERROR: Unrecognized action: '%c'\n", n.first);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char* argv[])
{
    int count=0;
    struct timespec start, stop;

    skiplist<int, int> *list = new skiplist<int, int>(0,1000000);

    // check and parse command line options
    if (argc != 3) {
        printf("Usage: %s <infile> <threadnum>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *fn = argv[1];
    int thread_num = atoi(argv[2]);

    clock_gettime( CLOCK_REALTIME, &start);

    // load input file
    FILE* fin = fopen(fn, "r");
    char action;
    long num;

    pair<char,long> temp;
    pthread_barrier_init(&b, NULL, thread_num + 1);
    pthread_t* threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);
    for(int i = 0 ; i < thread_num ; i++){
        pthread_create(&threads[i], NULL, worker, (void*)list);
    }

    while(fscanf(fin, "%c %ld\n", &action, &num) > 0) {
	if(action == 'p'){
	    print_flag = true;
	    pthread_cond_broadcast(&c);
	    pthread_barrier_wait(&b);
	    cout << list->printList() << endl;
	    print_flag = false;
	    pthread_barrier_wait(&b);
	}
	else{
	    temp = pair<char,long>(action,num);
	    pthread_mutex_lock(&m);
            q.push(temp);
	    pthread_mutex_unlock(&m);
            pthread_cond_signal(&c);
	}
	count++;	
    }
    flag = true;
    pthread_cond_broadcast(&c);

    for(int i =0; i< thread_num; i++){
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&c);
    pthread_barrier_destroy(&b);
    fclose(fin);
    clock_gettime( CLOCK_REALTIME, &stop);

    // print results
    double elapsed_time = (stop.tv_sec - start.tv_sec) + ((double) (stop.tv_nsec - start.tv_nsec))/BILLION ;

    cout << "Elapsed time: " << elapsed_time << " sec" << endl;
    cout << "Throughput: " << (double) count / elapsed_time << " ops (operations/sec)" << endl;

    return (EXIT_SUCCESS);
}



