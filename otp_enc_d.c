/*File:         otp_enc_d.c
 *Author:       Jiongcheng Luo
 *Date:         12/05/15
 *Description:  daemon/server/encryt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BUF_SIZE 102400

void error(const char *msg){
    perror(msg);
    exit(1);
}
/*###############################################################
                    MAIN
###############################################################*/
int main(int argc, char *argv[]){

    if (argc < 2) {
       fprintf(stderr,"ERROR, no port provided\n");
       exit(1);
    }
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    char msg_buffer[MAX_BUF_SIZE];
    char key_buffer[MAX_BUF_SIZE];
    char ciphertext[MAX_BUF_SIZE];
    int cofmmsg,msglen;
    int tmpNum,i;
    pid_t pid;

//##################### Setup Connection #####################
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);	//get port number
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
       error("ERROR on binding");
    listen(sockfd,5);	//listen port queue up to 5
    
//##################### Looping for communication #####################
    while(1){       
        clilen = sizeof(cli_addr);
    	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        bzero(msg_buffer,MAX_BUF_SIZE);
        bzero(key_buffer,MAX_BUF_SIZE);
        //sleep(1); //allow refreshing buffer
        if (newsockfd < 0) 
            error("ERROR on accept");      

    //##################### Check error from client input #####################
        cofmmsg=1;  //defualt true    
        //getting msg       
        n = read(newsockfd,msg_buffer,MAX_BUF_SIZE-1);
        if (n < 0) 
           error("ERROR reading from socket");
        int len = strlen(msg_buffer);

        if(msg_buffer[len-1] == 'e'){   //detect matched port from client, msg as 'e'
            msg_buffer[len-1] = 0;
            msglen = strlen(msg_buffer);
            //check msg input           
            for(i=0; i<msglen; i++){
                if(!(msg_buffer[i] >= 'A' && msg_buffer[i] <= 'Z' || msg_buffer[i] == ' ')){      //bad input
                    cofmmsg=0;    //plaintext error input
                    fprintf(stderr, "%s Error: input contains bad characters\n", argv[0]);
                    break;    
                }
            }               
        }else{
            fprintf(stderr, "Error: could not contact %s on port %d\n", argv[0], portno);
            cofmmsg=0;
        }
        //writing confirmation msg for valid input
        n = write(newsockfd, &cofmmsg, sizeof(cofmmsg));  
        if (n < 0) 
            error("ERROR writing to socket");
        //getting key
        n = read(newsockfd,key_buffer,MAX_BUF_SIZE-1);
        if (n < 0) 
            error("ERROR reading from socket");

    //##################### Encpypting  #####################
        pid = fork();
        if(pid == -1){   //error in fork();
            error("'fork()'");
            exit(1);
        }else if(pid == 0){ //child process            
            for(i=0; i<msglen; i++){
                tmpNum = msg_buffer[i] + key_buffer[i];
                if(tmpNum == 64){   //both are space
                    ciphertext[i] = 32;  
                }else if(msg_buffer[i] != 32 && key_buffer[i] == 32){     //key is space
                    ciphertext[i] = msg_buffer[i];
                }else if(msg_buffer[i] == 32 && key_buffer[i] != 32){     //msg of is space
                    ciphertext[i] = 32;
                }else{
                    if(tmpNum > 155)    //ciphertext > 26
                        ciphertext[i] = tmpNum - 91;
                    else
                        ciphertext[i] = tmpNum - 65;
                }
            }
            ciphertext[msglen]='\n';  
            n = write(newsockfd, ciphertext, strlen(ciphertext));
            if (n < 0) 
                error("ERROR writing to socket");
            close(newsockfd);
            exit(0);           
        }else{  //parent process
            close(newsockfd);
        }
    }  
    close(sockfd);
    return 0; 
}
