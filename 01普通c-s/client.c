#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "wrap.h"
#include <arpa/inet.h>

#define MAXLINE 80
//#define SERV_PORT 9999

int main(int argc, char *argv[])
{
    struct sockaddr_in servaddr;
    char buf[MAXLINE];
    int sockfd, n, port;
    char str[1024*4];

    if (argc != 2) {
        fputs("usage: ./client port\n", stderr);
        exit(1);
    }
    port = atoi(argv[1]);
	/*建立Socket*/
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	/*scoket结构体初始化*/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;							/*TCP/IP – IPv4*/
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);	/*iP转换*/
    servaddr.sin_port = htons(port);						/*设置端口*/

    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	printf("input send message:");
	scanf("%s",str);
    Write(sockfd, str, strlen(str));	/*发送数据，也可用send()*/

    n = Read(sockfd, buf, MAXLINE);		/*读取数据，也可用recv()*/
    printf("Response from server:\n");
    Write(STDOUT_FILENO, buf, n);		/*STDOUT_FILENO 标准输出(即屏幕)*/
	printf("\n");
    Close(sockfd);

    return 0;
}
