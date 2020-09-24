/* process_client.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include "wrap.h"
#include <arpa/inet.h>
#define MAXLINE 80
//#define SERV_PORT 8000

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;
	char buf[MAXLINE];
	int sockfd, n, port;
	if (argc != 2) {
        fputs("usage: ./client port\n", stderr);
        exit(1);
    }
	port = atoi(argv[1]);
	/*建立Socket*/
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	/*scoket结构体初始化*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;						/*TCP/IP – IPv4*/
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);/*IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。*/
	servaddr.sin_port = htons(port);					/*设置端口*/
	Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	printf("input recv message:\n");
	while (fgets(buf, MAXLINE, stdin) != NULL) {
		Write(sockfd, buf, strlen(buf));	/*发送数据，也可用send()*/
		n = Read(sockfd, buf, MAXLINE);		/*读取数据，也可用recv()*/
		if (n == 0) {
			printf("the other side has been closed.\n");
			break;
		} else
			Write(STDOUT_FILENO, buf, n);	/*STDOUT_FILENO 标准输出(即屏幕)*/
	}
	Close(sockfd);
	return 0;
}
