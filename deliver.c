#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>


#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server address> <server port number>\n");
        exit(1);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    struct timeval start, end;
    // opensocket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;// AF_INET = IPv4 addresses
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {//inet_pton convert an ip addr in nums and dots notation into a struct in6_addr(binary format), return -1 on error and 0 if the addr is messed up        perror("Invalid address");
        close(sockfd);
        exit(1);
    }

    // input message
    char input[BUF_SIZE];
    printf("Enter command (ftp <file name>): ");
    fgets(input, BUF_SIZE, stdin);
    char command[BUF_SIZE], filename[BUF_SIZE];

    if (sscanf(input, "%s %s", command, filename) != 2) {
        printf("Invalid input format.\n");
        close(sockfd);
        exit(1);
    }

    if (strcmp(command, "ftp") != 0) {
        printf("Invalid command. Only 'ftp' is supported.\n");
        exit(1);
    }
    struct stat st;
    if (stat(filename, &st) != 0) {
        perror("File does not exist"); //prints an error message describing why the file check failed
        close(sockfd); // closes the UDP socket before exiting
        exit(1);
    }
    gettimeofday(&start, NULL);
    // Send "ftp" message to server
    sendto(sockfd, "ftp", strlen("ftp"), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)); //sendto() is used with UDP (connectionless) to send a message.

    // Receive reply from server
    socklen_t addr_len = sizeof(server_addr);
    int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr*)&server_addr, &addr_len);
    if (n < 0) {
        perror("recvfrom failed");
        close(sockfd);
        exit(1);
    }
    gettimeofday(&end, NULL);

    buffer[n] = '\0';

    long sec = end.tv_sec - start.tv_sec;
    long usec = end.tv_usec - start.tv_usec;
    long rtt_us = sec * 1000000 + usec;

    printf("RTT = %.3f ms\n",
           rtt_us, rtt_us / 1000.0);

    if (strcmp(buffer, "yes") == 0) {
        printf("A file transfer can start.\n");
    } else {
        printf("Server denied file transfer.\n");
        close(sockfd);
        exit(1);
    }

    close(sockfd);
    return 0;
}
