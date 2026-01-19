#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

int main(int argc, char*argv[]){
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <UDP listen port>\n", argv[0]);
        return 1;
    }
   
   int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   if(sockfd==-1){
       printf("socket initialization fails\n");
   }
    
    struct sockaddr_in addr;
    int port = atoi(argv[1]);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MYPORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero,'\0', sizeof(addr.sin_zero)); //from Beej's Guide to Network Programming page 25
        
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr))==-1){
        printf("bind fails\n");
        return 1;
    }
   
    char buf[100]={0};
    struct sockaddr_in src_addr ={0};
    socklen_t src_addrlen = sizeof(src_addr);
    ssize_t receive = recvfrom (sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&src_addr, &src_addrlen);
    if(receive==-1){
        printf("receive error\n");
        return 1;
    }

    if(strcmp(buf,"ftp")==0){
        ssize_t send1 = sendto (sockfd, "yes", strlen("yes"), 0, (struct sockaddr*)&src_addr, src_addrlen);
        if(send1==-1){
            printf("send error\n");
            return 1;
        }
    }else{
        ssize_t send1 = sendto (sockfd, "no", strlen("no"), 0, (struct sockaddr*)&src_addr, src_addrlen);
        if(send1==-1){
            printf("send error\n");
            return 1;
        }
    }

    close(sockfd);
    return 0;
}
