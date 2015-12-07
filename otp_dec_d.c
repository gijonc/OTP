/*decrypt file (bg)
 *
 *
 *
 *
 *
 * */
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

    char cipher_buffer[MAX_BUF_SIZE];
    char key_buffer[MAX_BUF_SIZE];
    char ori_msg[MAX_BUF_SIZE];
    int cofmmsg,cipherlen,keylen;
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
        bzero(cipher_buffer,MAX_BUF_SIZE);
        bzero(key_buffer,MAX_BUF_SIZE);
        //sleep(1); //allow refreshing buffer
        if (newsockfd < 0) 
            error("ERROR on accept");
        
    //##################### Check cipher&port from client #####################
        cofmmsg=1;  //defualt true
        //getting cipher       
        n = read(newsockfd,cipher_buffer,MAX_BUF_SIZE-1);
        if (n < 0) 
           error("ERROR reading from socket");
        int len = strlen(cipher_buffer);

        if(cipher_buffer[len-1] == 'd'){   //detect matched port from client, msg as 'd'
            cipher_buffer[len-1] = 0;
            cipherlen = strlen(cipher_buffer);
            //check msg input           
            for(i=0; i<cipherlen; i++){
                if(!(cipher_buffer[i] >= 'A' && cipher_buffer[i] <= 'Z' || cipher_buffer[i] == ' ')){      //bad input
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
        
    //##################### Decpypting  #####################
        pid = fork();
        if(pid == -1){   //error in fork();
            error("'fork()'");
            exit(1);
        }else if(pid == 0){
            for(i=0; i<cipherlen; i++){
                tmpNum = cipher_buffer[i] - key_buffer[i];
                if(cipher_buffer[i]==' ' && key_buffer[i]==' '){   //both are space
                    ori_msg[i] = ' ';  
                }else if(cipher_buffer[i]!=' ' && key_buffer[i]==' '){     //key is space
                    ori_msg[i] = cipher_buffer[i];
                }else if(cipher_buffer[i]==' ' && key_buffer[i]!=' '){     //cipher is space
                    ori_msg[i] = ' ';
                }else{
                    if(tmpNum < 0)    //ciphertext > 26
                        ori_msg[i] = tmpNum + 91;
                    else
                        ori_msg[i] = tmpNum + 65;
                }
            }
            ori_msg[cipherlen]='\n';          
            n = write(newsockfd,ori_msg,strlen(ori_msg));
            if (n < 0) 
                error("ERROR writing to socket");
            close(newsockfd);  
            exit(0);
        }else{
            close(newsockfd);
        }
    }  
    close(sockfd);
    return 0; 
}