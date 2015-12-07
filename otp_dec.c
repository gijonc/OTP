/*File:         otp_edec.c
 *Author:       Jiongcheng Luo
 *Date:         12/05/15
 *Description:  client/decryt
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define MAX_BUF_SIZE 102400
static const char hostname[]="localhost";

void read_file(char*,char*,int);
void error(const char *msg){
    perror(msg);
    exit(1);
}

/*###############################################################
								            MAIN
###############################################################*/
int main(int argc, char **argv)
{
//##################### Check Inputs #####################		
	if(argc < 4){				//check at least 4 arguments
      fprintf(stderr,"ERROR: Missing inputs to %s\n", argv[0]);
		  exit(1);
    }
    char ciphertext[MAX_BUF_SIZE];
    char key[MAX_BUF_SIZE];
    char ori_msg[MAX_BUF_SIZE];
    int cnfmmsg=0,cipherlen,keylen;

    int sockfd, portno, n, i;
    struct sockaddr_in serv_addr;
    struct hostent *server;

//##################### Reading files and info ####################   
    read_file(argv[1],ciphertext,1);
    read_file(argv[2],key,0);
    cipherlen = strlen(ciphertext);
    keylen = strlen(key);  
    if(keylen < cipherlen){    //check length of key
        fprintf(stderr, "Key '%s' is too short\n", argv[2]);
        exit(1);
    }

//##################### Setup Connection #####################
    portno = atoi(argv[3]);   //get portno from third argc
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(hostname);
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

  //##################### writing & reading to socket ####################   
    //write msg to socket
    n = write(sockfd, ciphertext, strlen(ciphertext));
    if (n < 0) 
        error("ERROR writing to socket");

    //receiving confirmation msg from server
    n = read(sockfd, &cnfmmsg, sizeof(cnfmmsg));
    if (n < 0) 
        error("ERROR reading from socket");
    if(cnfmmsg != 1){ //confirm error input from server
        exit(1);
    }

    //write key to socket
    n = write(sockfd,key,strlen(key));
    if (n < 0) 
        error("ERROR writing to socket");

    //receiving original msg from server
    n = read(sockfd,ori_msg,MAX_BUF_SIZE-1);
    if (n < 0) 
         error("ERROR reading from socket");
    fprintf(stdout,"%s",ori_msg);

    close(sockfd);
    return 0;
}

void read_file(char *file_path, char *buffer, int id){
  int leng = 0;  //length of line
  FILE *file;

  file = fopen(file_path, "r");   
    if(file == NULL){               //check error in opening file_1
      fprintf(stderr,"Error opening '%s': No such file or directory\n",file_path);
      exit(1);
    }
    while(fgets(buffer, MAX_BUF_SIZE, file) != NULL) {     //fgets each buffer until end
      if (buffer[0] != '\n'){      //detect any emtpy buffer
          leng = strlen(buffer);
          if (leng > 0 && buffer[leng-1] == '\n'){   //check new buffer char \r\n
            if(id==1)
              buffer[leng-1] = 'd';   //buffer as ciphertext, set d as id
            else
              buffer[leng-1] = 0;
          }
      }
    fclose(file);  
    }  
}