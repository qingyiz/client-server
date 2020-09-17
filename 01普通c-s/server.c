#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "wrap.h"

#define MAXLINE 80
//#define SERV_PORT 6666

int main(int argc, char *argv[])
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    int i, n, port;
	if (argc != 2) {
        fputs("usage: ./server port\n", stderr);
        exit(1);
    }
	port = atoi(argv[1]);
	/*建立Socket*/
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	/*scoket结构体初始化*/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;					/*TCP/IP – IPv4*/
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	/*IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。*/
    servaddr.sin_port = htons(port);				/*设置端口*/

    
    /*设置端口复用*/
    /*int opt = 1; 
    setsockopt(listenfd, SOL_SOCKET,
               SO_REUSEPORT,
               (const void *)&opt, 
               sizeof(opt)
               );*/
    
    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));	/*将本地地址绑定到所创建的套接字上*/
    Listen(listenfd, 20);											/*开始监听是否有客户端连接*/

    printf("Accepting connections ...\n");
    while (1) {
        cliaddr_len = sizeof(cliaddr);
        connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);	 /*阻塞直到有客户端连接，不然多浪费CPU资源。*/
        n = Read(connfd, buf, MAXLINE);		/*读取数据，也可用recv()*/
        printf("received from %s at PORT %d\n",
               inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),			/*获取ip地址*/
               ntohs(cliaddr.sin_port));										/*获取端口*/
		printf("recv message:%s\n",buf);
        for (i = 0; i < n; i++)
            buf[i] = toupper(buf[i]);
        Write(connfd, buf, n);		/*发送数据，也可用send()*/
        close(connfd);
    }
    return 0;
}
