#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024

void error_handling(char *message);

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
        printf("-----File list-----\n");

        while(1)
        {
            read_cnt = read(sock, buf, BUF_SIZE);
            if(read_cnt == -1)
                error_handling("read() error");

            if(strcmp(buf, "end") == 0 || read_cnt <= 0)
                break;

            printf("%s\n", buf);
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

        // 파일 수신
        read(sock, &filesize, sizeof(filesize));
        filesize = htonl(filesize);
        file = fopen(file_name, "wb");
        if(file == NULL) {
            printf("Failed to create file.\n");
            continue;
        }

        nbyte = BUF_SIZE;
        while(filesize > 0)
        {
            if(filesize < BUF_SIZE)
                nbyte = read(sock, buf, filesize);
            else
                nbyte = read(sock, buf, BUF_SIZE);

            if(nbyte == -1)
            {
                error_handling("read() error");
                break;
            }

            fwrite(buf, sizeof(char), nbyte, file);
            filesize -= nbyte;
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