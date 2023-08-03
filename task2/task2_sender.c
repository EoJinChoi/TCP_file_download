#define _XOPEN_SOURCE 200

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>

#define BUF_SIZE 1024
void error_handling(char *message);
int ack = 0;

typedef struct {
    int seq;
    int ack;
}pkt;
pkt *recv_pkt;

typedef struct {
    int seq;
    char fileName[BUF_SIZE];
    char fileSize[BUF_SIZE];
    char fileContent[BUF_SIZE];
    int size;
} pkt_t;
pkt_t *data_pkt;

void timeout(int sig)
{
    if(sig == SIGALRM)
        puts("timeout");
}

int main(int argc, char *argv[])
{
    int sock;
    socklen_t adr_sz;
    FILE *file;
    int seq = 0;
    char file_name[BUF_SIZE] = "photo.jpeg";
    int fpsize;

    data_pkt = (pkt_t *) malloc(sizeof(pkt_t));
    memset(data_pkt, 0, sizeof(pkt_t));
    recv_pkt = (pkt *) malloc(sizeof(pkt));
    memset(recv_pkt, 0, sizeof(pkt));

    struct sigaction act;
    act.sa_handler = timeout;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGALRM, &act, 0);

    struct sockaddr_in serv_adr, from_adr;
    if(argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    strcpy(data_pkt->fileName, file_name);

    file = fopen(file_name, "rb");
    if(file == NULL) {
        printf("Failed to open file.\n");
        exit(1);
    }

    size_t fsize, nsize = 0;

    //파일 크기 구하기
    fseek(file, 0, SEEK_END);
    fsize=ftell(file);
    fseek(file, 0, SEEK_SET);

    char size[BUF_SIZE];
    snprintf(size, sizeof(size), "%zu", fsize);
    strcpy(data_pkt->fileSize, size);

    clock_t start, end;
    // File write
    while(nsize != fsize)
    {
        printf("read\n");
        ack = 0;
        data_pkt->seq = seq;
        printf("fread\n");
        fpsize = fread(data_pkt->fileContent, 1, BUF_SIZE, file);
        printf("read complete\n");

        char size[BUF_SIZE];
        snprintf(size, sizeof(size), "%d\n", fpsize);
        strcpy(data_pkt->fileSize, size);

        int read;
        while(1)
        {
            printf("send start\n");
            if(fpsize < BUF_SIZE)
            {
                data_pkt->size = fpsize;
                alarm(1);
                sendto(sock, data_pkt, sizeof(pkt_t), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
                read = recvfrom(sock, recv_pkt, sizeof(pkt), 0, (struct sockaddr*)&from_adr, &adr_sz);
                if(seq == 0)
                    start = clock();
                else
                    end = clock();
                if(read == -1 || recv_pkt->ack == 0) // file이 잘 전송되지 않았을 때
                    continue;
            }
            else
            {
                data_pkt->size = fpsize;
                alarm(1);

                sendto(sock, data_pkt, sizeof(pkt_t), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
                read = recvfrom(sock, recv_pkt, sizeof(pkt), 0, (struct sockaddr*)&from_adr, &adr_sz);
                if(seq == 0)
                    start = clock();
                else
                    end = clock();
                if(read == -1 || recv_pkt->ack == 0) // file이 잘 전송되지 않았을 때
                    continue;   
            }                              
            break;
        }
        seq++;
        nsize += fpsize;

        if(read > 0)
            printf("send complete %d\n", seq); 
    }
    printf("\ntime: %lf\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("file size: %ld\n", fsize);
    printf("Throughput: %f b/s\n", fsize / (double)(end - start));

    fclose(file);
    printf("File send complete\n");

    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}