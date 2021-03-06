/* server.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include "wrap.h"
#include <ctype.h>
#define MAXLINE 80
#define SERV_PORT 6666
#define OPEN_MAX 1024

int main(int argc, char *argv[])
{
	int i, j, maxi, listenfd, connfd, sockfd, port;
	int nready;
	ssize_t n;
	char buf[MAXLINE], str[INET_ADDRSTRLEN];
	socklen_t clilen;
	struct pollfd client[OPEN_MAX];
	struct sockaddr_in cliaddr, servaddr;
	if (argc != 2) {
        fputs("usage: ./server port\n", stderr);
        exit(1);
    }
	port = atoi(argv[1]);

	/* 创建套接字*/
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	/* 初始化服务器 sockaddr_in */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;					/* 地址族 */
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	/* 监听本机所有的IP*/
	servaddr.sin_port = htons(port);			/* 设置端口*/ 

	/* 绑定IP和端口*/
	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	
	/* 设置同时监听的最大个数*/
	Listen(listenfd, 20);
	printf("Start accept ......\n");

	client[0].fd = listenfd;
	client[0].events = POLLRDNORM; 					/* listenfd监听普通读事件 */
	
	/*指定为-1，规定-1为未使用*/
	for (i = 1; i < OPEN_MAX; i++)
		client[i].fd = -1; 						
	maxi = 0; 										/* client[]数组有效元素中最大元素下标 */

	while (1){
		
		nready = poll(client,maxi+1,-1); 			/* 阻塞 */
		if (client[0].revents & POLLRDNORM) { 		/* 有客户端链接请求 */
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
			printf("received from %s at PORT %d\n",
					inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
					ntohs(cliaddr.sin_port));
					
			for (i = 1; i < OPEN_MAX; i++) {
				if (client[i].fd < 0) {
					client[i].fd = connfd; 	/* 找到client[]中空闲的位置，存放accept返回的connfd */
					break;
				}
			}

			if (i == OPEN_MAX)
				perr_exit("too many clients");

			client[i].events = POLLRDNORM; 		/* 设置刚刚返回的connfd，监控读事件 */
			if (i > maxi)
				maxi = i; 						/* 更新client[]中最大元素下标 */
			if (--nready <= 0)
				continue; 						/* 没有更多就绪事件时,继续回到poll阻塞 */
		}
		for (i = 1; i <= maxi; i++) { 			/* 检测client[] */
			if ((sockfd = client[i].fd) < 0)
				continue;
			if (client[i].revents & (POLLRDNORM | POLLERR)) {
				if ((n = Read(sockfd, buf, MAXLINE)) < 0) {
					if (errno == ECONNRESET) { /* 当收到 RST标志时 */
						/* connection reset by client */
						printf("client[%d] aborted connection\n", i);
						Close(sockfd);
						client[i].fd = -1;
					} else {
						perr_exit("read error");
					}
				} else if (n == 0) {
					/* connection closed by client */
					printf("client[%d] 断开了连接\n", i);
					Close(sockfd);
					client[i].fd = -1;
				} else {
					printf("recv buf: %s\n", buf);
					for (j = 0; j < n; j++)
						buf[j] = toupper(buf[j]);
						Writen(sockfd, buf, n);
				}
				if (--nready <= 0)
					break; 				/* no more readable descriptors */
			}
		}
	}
	return 0;
}
