#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>
#include "wrap.h"

#define MAXLINE 80
#define SERV_PORT 6666
#define OPEN_MAX 1024

int main(int argc, char *argv[])
{
	int i, j, maxi, listenfd, connfd, sockfd,port;
	int nready, efd, res;
	ssize_t n;
	char buf[MAXLINE], str[INET_ADDRSTRLEN];
	socklen_t clilen;
	int client[OPEN_MAX];
	struct sockaddr_in cliaddr, servaddr;
	struct epoll_event tep, ep[OPEN_MAX];
	if (argc != 2) {
        fputs("usage: ./server port\n", stderr);
        exit(1);
    }
	port = atoi(argv[1]);
	
	/*建立Socket*/
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	/*scoket结构体初始化*/
	bzero(&servaddr, sizeof(servaddr));				/*清空结构体，相当于memset()*/
	servaddr.sin_family = AF_INET;					/*TCP/IP – IPv4*/
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	/*IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。*/		
	servaddr.sin_port = htons(port);			/*设置端口*/

	Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));	/*将本地地址绑定到所创建的套接字上*/

	Listen(listenfd, 20);	/*开始监听是否有客户端连接*/

	/*指定为-1，规定-1为未使用*/
	for (i = 0; i < OPEN_MAX; i++)
		client[i] = -1;
	maxi = -1;
	/*创建epoll树根节点*/
	efd = epoll_create(OPEN_MAX);
	if (efd == -1)
		perr_exit("epoll_create");
	
	/*初始化epoll树*/
	tep.events = EPOLLIN; /*设置要处理的事件类型*/
	
	tep.data.fd = listenfd;/*设置与要处理的事件相关的文件描述符*/
	
	/*注册epoll事件，EPOLL_CTL_ADD 表示 注册新的fd到efd*/
	res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
	if (res == -1)
		perr_exit("epoll_ctl");

	while (1) {
		nready = epoll_wait(efd, ep, OPEN_MAX, -1); /* 阻塞监听 */
		if (nready == -1)
			perr_exit("epoll_wait");

		for (i = 0; i < nready; i++) {
			if (!(ep[i].events & EPOLLIN))
				continue;
			/* 判断是否有新连接*/
			if (ep[i].data.fd == listenfd) {
				clilen = sizeof(cliaddr);
				connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
				 /*打印客户端信息*/
				printf("received from %s at PORT %d\n", 
						inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), 
						ntohs(cliaddr.sin_port));
				for (j = 0; j < OPEN_MAX; j++) {
					if (client[j] < 0) {/*即为-1时，为未使用*/
						client[j] = connfd; /* save descriptor */
						break;
					}
				}

				if (j == OPEN_MAX)
					perr_exit("too many clients");
				if (j > maxi)
					maxi = j; 		/*更新最大连接数*/
				
				/*把新的连接挂载到epoll*/
				tep.events = EPOLLIN; 	
				tep.data.fd = connfd;
				res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
				if (res == -1)
					perr_exit("epoll_ctl");
			} 
			/*断开连接和正常接收*/
			else {
				sockfd = ep[i].data.fd;
				n = Read(sockfd, buf, MAXLINE);
				/*断开连接*/
				if (n == 0) {
					for (j = 0; j <= maxi; j++) {
						if (client[j] == sockfd) {
							client[j] = -1;
							break;
						}
					}
					res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
					if (res == -1)
						perr_exit("epoll_ctl");

					Close(sockfd);
					printf("client[%d] closed connection\n", j);
				} 
				/*正常接收*/
				else {
					printf("recv message:%s",buf);
					for (j = 0; j < n; j++)
						buf[j] = toupper(buf[j]);
					Writen(sockfd, buf, n);
				}
			}
		}
	}
	Close(listenfd);
	Close(efd);
	return 0;
}
