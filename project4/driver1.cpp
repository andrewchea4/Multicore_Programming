#include <stdlib.h>
#include <stdio.h>
#include <cuda_runtime.h>
#include <unistd.h>
#include <iostream>

#define N 1000000000
#define MAX_VAL 1000000000

extern void counting_sort(int arr[], int, int);

int main()
{
  static struct timespec begin, end;
  clock_gettime(CLOCK_REALTIME, &begin);
  //int array[N];
  int *array = (int *)malloc(sizeof(int) * N);

  for(int i=0;i<N;i++){
      array[i] = rand()%MAX_VAL;
      //printf("%d\t", array[i]);
  }

  counting_sort(array, N, MAX_VAL);

  for(int i=0;i<N-1;i++){
      if( array[i] > array[i+1]){
          printf("Not sorted\n");
          exit(1);
      }
  }

//  printf("AFTER SORT\n");
//  for(int i =0; i<N; i++){
//	  printf("%d\t", array[i]);
 // }
  
  printf("Sorted\n");
  clock_gettime(CLOCK_REALTIME, &end);
  long time = 1000000000*(end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec);
  printf("(Nano): %ld\n", time);
  printf("(Micro): %lf\n", (double)time/1000);
  printf("(Milli): %lf\n", (double)time/1000000);

}
