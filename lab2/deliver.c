#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>

#define DATA_SIZE 1000
#define BUF_SIZE 2048
#define TIMEOUT_SEC 2
#define MAX_RETRIES 3

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server address> <server port>\n", argv[0]);
        exit(1);
    }

    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    //Set receive timeout
    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(1);
    }

    //Configure server address
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(1);
    }

    //Get file name
    char filename[256];
    printf("Enter filename: ");
    fgets(filename, sizeof(filename), stdin);
    
    // Get rid of \0
    filename[strcspn(filename, "\n")] = '\0';
    
    if (strlen(filename) == 0) {
        printf("No filename provided\n");
        close(sockfd);
        exit(1);
    }

    //Check if file exist
    struct stat st;
    if (stat(filename, &st) != 0) {
        perror("File does not exist");
        close(sockfd);
        exit(1);
    }

    //Open file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Cannot open file");
        close(sockfd);
        exit(1);
    }
    
    //Get size of file
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    int total_frag = (file_size + DATA_SIZE - 1) / DATA_SIZE;
    char data[DATA_SIZE];

    printf("Sending file: %s (%d bytes, %d fragments)\n", filename, file_size, total_frag);

    //Sending fragments
    for (int frag_no = 1; frag_no <= total_frag; frag_no++) {
        int size = fread(data, 1, DATA_SIZE, file);
        
        char info[512];
        int info_len = sprintf(info, "%d:%d:%d:%s:", total_frag, frag_no, size, filename);

        char *packet = malloc(info_len + size);
        if (!packet) {
            perror("malloc");
            fclose(file);
            close(sockfd);
            exit(1);
        }
        memcpy(packet, info, info_len);
        memcpy(packet + info_len, data, size);

        //Wait for ACK
        int retries = 0;
        int ack_received = 0;
        
        while (retries < MAX_RETRIES && !ack_received) {
            // Send packets
            ssize_t sent = sendto(sockfd, packet, info_len + size, 0,
                                  (struct sockaddr *)&server_addr, addr_len);
            if (sent < 0) {
                perror("sendto");
                break;
            }


            //get ACK
            char ack_buf[16];
            ssize_t n = recvfrom(sockfd, ack_buf, sizeof(ack_buf), 0,
                                 (struct sockaddr *)&server_addr, &addr_len);
                        
            if (n > 0) {
            
                ack_buf[n] = '\0';
            
                // Check if "ACK"
                if (strcmp(ack_buf, "ACK") == 0) {
                    ack_received = 1;
                    printf("Fragment %d acknowledged\n", frag_no);
                } else {
                    printf("Invalid response: '%s'. Expected 'ACK'. Resending...\n", ack_buf);
                    retries++;
                }
                
            } else {
                // Timeout retry
                printf("Timeout for fragment %d, retrying...\n", frag_no);
                retries++;
            }
        }
        
        free(packet);
        
        if (!ack_received) {
            printf("Failed to send fragment %d after %d retries\n", 
                   frag_no, MAX_RETRIES);
            fclose(file);
            close(sockfd);
            return 1;
        }
    }


    fclose(file);
    close(sockfd);
    printf("File transfer completed successfully\n");
    return 0;
}


































































































