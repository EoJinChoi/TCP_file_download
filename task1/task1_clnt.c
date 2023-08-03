/*
TCP 기반 파일 다운로드 프로그램 구현
• 1. 클라이언트가 서버에 접속 (TCP 이용)
• 2.서버프로그램이실행중인디렉토리의모든파일목록(파일이름, 파일 크기)을 클라이언트에게 전송
• 3.클라이언트는서버가보내온목록을보고파일하나를선택
• 4. 서버는 클라이언트가 선택한 파일을 클라이언트에게 전송
• 5. 전송된 파일은 클라이언트 프로그램이 실행 중인 디렉토리에 동일한 이름으로 저장됨.
• 6. 2~5번 과정 반복
• 사용자Interface는자유롭게해도됨.단,사용하기 쉽도록 메뉴나 명령어에 대한 설명 필요
• 텍스트 파일 뿐만 아니라 바이너리 파일도 전송할 수 있어야 함
*/

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
pkt_t *recv_pkt;

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

    recv_pkt = (pkt_t *) malloc(sizeof(pkt_t));
    memset(recv_pkt, 0, sizeof(pkt_t));

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
            int read_size = 0;

            while(1)
            {
                read_cnt = recv(sock, recv_pkt, sizeof(pkt_t), MSG_PEEK);
                if(read_cnt == sizeof(pkt_t))
                    break;
            }
            recv(sock, recv_pkt, sizeof(pkt_t), 0);

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
            while(1)
            {
                read_cnt = recv(sock, recv_pkt, sizeof(pkt_t), MSG_PEEK);
                if(read_cnt == sizeof(pkt_t))
                    break;

                printf("\nsize: %d\n", read_cnt);
            }
            recv(sock, recv_pkt, sizeof(pkt_t), 0);
            fwrite(recv_pkt->fileContent, 1, recv_pkt->size, file);

            if(recv_pkt->size < BUF_SIZE)
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