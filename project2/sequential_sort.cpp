#include<iostream>
#include<fstream>
#include<string.h>

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

    static struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);

    for(i=1; i<N; i++)
    {
        for(j=1; j<N; j++)
        {
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

    delete[] strArr;
    clock_gettime(CLOCK_REALTIME, &end);
    long time = 1000000000*(end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec);
    printf("(Nano): %ld\n", time);
    printf("(Micro): %lf\n", (double)time/1000);
    printf("(Milli): %lf\n", (double)time/1000000);


    return 0;
}
