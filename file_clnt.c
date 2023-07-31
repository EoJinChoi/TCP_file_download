#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024

void error_handling(char *message);

typedef struct {
    char fileName[BUF_SIZE];
    char fileSize[BUF_SIZE];
    char fileContent[BUF_SIZE];
    int size;
} pkt_t;

int main(int argc, char *argv[])
{
    int sock;
    char buf[BUF_SIZE];
    char file_name[BUF_SIZE];
    int read_cnt, menu = 0;
    struct sockaddr_in serv_addr;
    FILE *file;
    int nbyte;
    size_t filesize = 0, bufsize = 0;

    pkt_t *recv_pkt = (pkt_t *) malloc(sizeof(pkt_t));//
    memset(recv_pkt, 0, sizeof(pkt_t));//

    if(argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        error_handling("socket() error");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");

    while(1)
    {
        // File list 출력
        printf("-----File list-----\n");

        while(1)
        {
            read_cnt = read(sock, recv_pkt, sizeof(pkt_t));
            if(read_cnt == -1)
                error_handling("read() error");

            if(strcmp(recv_pkt->fileName, "end") == 0 || read_cnt <= 0)
                break;

            printf("%s   ", recv_pkt->fileName);
            printf("%s bytes\n", recv_pkt->fileSize);
        }

        printf("-------------------\n");

        // Menu 선택
        printf("1. Download\n");
        printf("2. Quit\n");
        printf("Enter number : ");
        scanf("%d", &menu);

        if(menu == 2) break;

        // 선택한 파일 이름 write
        printf("\nChoose one file : ");
        scanf("%s", file_name);
        if(strlen(file_name) >= BUF_SIZE) {
            printf("Invalid file name (too long).\n");
            continue;
        }

        write(sock, file_name, sizeof(file_name));

        // File write
        file = fopen(file_name, "wb");
            if(file == NULL) {
                printf("Failed to create file.\n");
                continue;
            }
        while(1)
        {
            read(sock, recv_pkt, sizeof(pkt_t));//

            printf("\nsize: %zu\n", filesize);
            fwrite(recv_pkt->fileContent, 1, recv_pkt->size, file);

            if(recv_pkt->size <BUF_SIZE)
                break;
        }

        fclose(file);
        printf("receive complete\n");
    }

    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}