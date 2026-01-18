#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char*argv[]){
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   
    int bindReturnValue = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (bindReturnValue==-1){
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


    return 0;
}
