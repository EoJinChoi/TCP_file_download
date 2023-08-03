/*
Simple Remote Shell 프로그램 구현
• 클라이언트가 서버에 접속하여 서버의 디렉토리와 파일 정보를 확인할 수 있는 프로그램을 작성하세요.
• 클라이언트가 서버에 접속하면, 서버는 서버프로그램이 실행되고 있는 디렉토리 위치 및 파일 정보(파일명 + 파일 크기)를 클라이언트에게 전달합니다.
• 클라이언트는 서버로부터 수신한 정보들을 출력합니다.
• 클라이언트는 서버의 디렉토리를 변경해가며 디렉토리 정보와 디렉토리 안에 있는 파일 정보(파일명 + 파일크기)를 확인할 수 있습니다.
• 클라이언트가 원하는 파일을 서버로부터 다운로드받을 수 있어야 합니다.
• 클라이언트는 자신이 소유한 파일을 서버에게 업로드할 수 있어야 합니다.
• 클라이언트가 서버에 접속하면 클라이언트는 서버프로그램의 권한과 동일한 권한을 갖습니다. 즉, 서버 프로그램에게 허용되지 않은 디렉토리와 파일은 클라이언트도 접근할 수 없습니다.
• 서버는 여러 클라이언트의 요청을 동시에 지원할 수 있어야 합니다. →I/O Multiplexing 사용!
• Makefile을 만들어서 컴파일할 수 있어야 합니다.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dirent.h>
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
    DIR* dir;
    struct dirent* entry;
    char fp[BUF_SIZE];

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

    char msg[BUF_SIZE] = "start";

    while(1)
    {
        write(sock, msg, sizeof(msg));
        printf("hello\n");
        read(sock, buf, BUF_SIZE);
        printf("----------------------------------------------------------\n");
        printf("Folder path: %s\n", buf);
        printf("----------------------------------------------------------\n");
        while(1)
        {
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
        printf("----------------------------------------------------------\n");

        // Menu 선택
        printf("1. Change directory\n");
        printf("2. Download\n");
        printf("3. Upload\n");
        printf("4. Quit\n");
        printf("Enter number : ");
        scanf("%d", &menu);
        getchar();

        char num[10];
        snprintf(num, sizeof(num), "%d", menu);
        write(sock, &menu, sizeof(int));
        if(menu == 4) break;

        // 1. Change directory---------------------------
        if(menu == 1)
        {
            printf("Folder path: ");
            scanf("%s", fp);
            write(sock, fp, sizeof(fp));
            continue;
        }
        // 2. Download-----------------------------------
        else if(menu == 2)
        {
            // 파일 선택
            printf("\nChoose one file : ");
            scanf("%s", file_name);
            if(strlen(file_name) >= BUF_SIZE) {
                printf("Invalid file name (too long).\n");
                continue;
            }

            write(sock, file_name, sizeof(file_name));

            // File read
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

        // 3. Upload------------------------------------
        else if(menu == 3)
        {
            char* folderPath = ".";

            dir = opendir(folderPath);
            if(dir == NULL) {
                printf("Failed to open directory.\n");
            }

            sleep(1);
            printf("\nFile list-----------------------------------------------\n");
            while((entry = readdir(dir)) != NULL) {
                if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;
                printf("%s   ", entry->d_name);

                // 파일 크기 구하기
                file = fopen(entry->d_name, "rb");
                size_t fsize;
                fseek(file, 0, SEEK_END);
                fsize=ftell(file);
                fseek(file, 0, SEEK_SET);
                fclose(file);

                printf("%zu bytes\n", fsize);
            }
            printf("--------------------------------------------------------\n");


            printf("Choose one file : ");
            scanf("%s", file_name);
            getchar();
            
            write(sock, file_name, sizeof(file_name));
            
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
            strcpy(recv_pkt->fileSize, size);

            // File write
            while(nsize != fsize)
            {
                int fpsize = fread(buf, 1, BUF_SIZE, file);
                nsize += fpsize;
                                
                for(int i = 0; i < fpsize; i++)
                    recv_pkt->fileContent[i] = buf[i];

                if(fpsize < BUF_SIZE)
                {
                    recv_pkt->size = fpsize;
                    write(sock, recv_pkt, sizeof(pkt_t));
                    break;
                }
                                
                recv_pkt->size = fpsize;
                write(sock, recv_pkt, sizeof(pkt_t));
            }

            fclose(file);
            printf("file send complete\n");
        }
    }
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}