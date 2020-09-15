/*process_server.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "wrap.h"

#define MAXLINE 80
//#define SERV_PORT 8000

void do_sigchild(int num)
{
	while (waitpid(0, NULL, WNOHANG) > 0)
		;
}
int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	int listenfd, connfd;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];
	int i, n, port;
	pid_t pid;
	if (argc != 2) {
        fputs("usage: ./server port\n", stderr);
        exit(1);
    }
	port = atoi(argv[1]);
	
	/*sigaction结构体初始化，用来回收子进程pcb*/
	struct sigaction newact;
	newact.sa_handler = do_sigchild;	/*回调*/
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGCHLD, &newact, NULL);
	
	/*建立Socket*/
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	/*scoket结构体初始化*/
	bzero(&servaddr, sizeof(servaddr));				/*清空结构体，相当于memset()*/
	servaddr.sin_family = AF_INET;					/*TCP/IP – IPv4*/
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	/*IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。*/
	servaddr.sin_port = htons(port);				/*设置端口*/

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));	/*将本地地址绑定到所创建的套接字上*/

	Listen(listenfd, 20);		/*开始监听是否有客户端连接*/

	printf("Accepting connections ...\n");
	while (1) {
		cliaddr_len = sizeof(cliaddr);
		connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len); /*父进程接受连接请求*/

		pid = fork();
		if (pid == 0) { /*子进程，关闭父进程监听socket*/
			Close(listenfd);
			while (1) {
				n = Read(connfd, buf, MAXLINE);
				if (n == 0) {
					printf("客户端断开连接 .\n");
					break;
				}
				printf("received from %s at PORT %d\n",
						inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
						ntohs(cliaddr.sin_port));
				printf("received message:%s",buf);
				for (i = 0; i < n; i++)
					buf[i] = toupper(buf[i]);
				Write(connfd, buf, n);
			}
			Close(connfd);
			
			return 0;	/*关闭子进程*/
		} else if (pid > 0) {
			Close(connfd);
		} else
			perr_exit("fork");
	}
	Close(listenfd);
	return 0;
}
