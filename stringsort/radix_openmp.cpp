#include<iostream>
#include<fstream>
#include<string.h>
#include <omp.h>
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

    auto strArr = new char[N][30];

    for(i=0; i<N; i++)
        inputfile>>strArr[i];

    inputfile.close();

    int sum = 0;
    int tmp_sum = 0;
//omp_set_nested(true);

#pragma omp parallel num_threads(16) shared(sum)
    {
#pragma omp for
		    for(int i = 0; i< N; i++){
			    if(strlen(strArr[i]) > tmp_sum)
				    tmp_sum = strlen(strArr[i]);
		    }
#pragma omp barrier
#pragma omp critical
	    {
		    if(sum < tmp_sum)
			    sum = tmp_sum;
	    }
    }
    cout << "sum : " << sum << endl;

    cout<<"\nStrings (Names) in Alphabetical order from position " << pos << ": " << endl;
    for(i=pos; i<N && i<(pos+range); i++)
        cout<< i << ": " << strArr[i]<<endl;
    cout<<endl;

    delete[] strArr;

    return 0;
}
