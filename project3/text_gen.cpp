#include<iostream>
#include<fstream>
#include<string.h>

using namespace std;

int main(int argc, char* argv[])
{
    char tmpStr[30];
    int i, j, N, ret;

    if(argc<3){
	    cout << "Usage: " << argv[0] << " filename number_of_strings" << endl;
	    return 0;
    }

    ofstream outputfile(argv[1]);
    if(!outputfile.is_open()){
	    cout << "Unable to open file" << endl;
	    return 0;
    }

    ret=sscanf(argv[2],"%d", &N);
    if(ret==EOF || N<=0){
	    cout << "Invalid number" << endl;
	    return 0;
    }

    int min=65;
    int max=122;
    int strlen_prob=4;

    srand( (unsigned) time(0));
    for(i=0; i<N; i++) {
        for(j=0; j<30; j++) {
		int r = rand()%(max-min) + min;
		tmpStr[j] = r;
		if(j<29 && rand()%strlen_prob==0) {
			tmpStr[j+1] = '\0';
			break;
		}
	}
        outputfile << tmpStr << " ";
	if(rand()%20==0) outputfile << endl;

	if(strlen(tmpStr)>30) cout << tmpStr << endl;
    }

    outputfile.close();

    return 0;
}
