/*	keygen.c
	creates a key file
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <unistd.h>
#include <fcntl.h>

static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

int main(int argc, char *argv[]){
	
	if(argc < 2){	//take 2 argvs
		fprintf(stderr,"ERROR: Invalid inputs to %s\n", argv[0]);
		exit(0);
	}
	int n,i,length,fd;
	srand (time(NULL));
	FILE *file;
	length = atoi(argv[1]);	//take input key length
	char key[length];
	for (i = 0; i < length; i++) {
        key[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    key[length]='\n';
   	fprintf(stdout, "%s", key);
	return 0;
}