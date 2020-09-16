/* pthread_server.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "wrap.h"
#include <pthread.h>

#define MAXLINE 80
#define SERV_PORT 6666

/*自定义数据结构，用来线程函数传参*/
struct s_info {
	struct sockaddr_in cliaddr;
	int connfd;
	pthread_t pid;
};
void *do_work(void *arg)
{
	int n,i;
	struct s_info *ts = (struct s_info*)arg;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];
	
	
	while (1) {
		n = Read(ts->connfd, buf, MAXLINE);
		if(n == -1){
			perror("read error\n");
			pthread_exit(NULL);
		}
		else if (n == 0) {
			Close(ts->connfd);
			printf("客户端断开连接 .\n");
			break;
		}
		printf("received from %s at PORT %d\n",
				inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str)),
				ntohs((*ts).cliaddr.sin_port));
		for (i = 0; i < n; i++)
			buf[i] = toupper(buf[i]);
		Write(ts->connfd, buf, n);
	}
	Close(ts->connfd);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	int i = 0,port;
	int listenfd;
	struct s_info ts[256];
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
	servaddr.sin_port = htons(port);				/*设置端口*/

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));	/*将本地地址绑定到所创建的套接字上*/
	Listen(listenfd, 20);		/*开始监听是否有客户端连接*/

	/*规定结构体的pid默认为-1，好判断该内存是否被使用*/
	for(i = 0;i<sizeof(ts)/sizeof(ts[0]);i++){
		ts[i].pid = -1;
	}
	cliaddr_len = sizeof(cliaddr);
	printf("Accepting connections ...\n");
	while (1) {
		/* 选一个没有被使用的, 最小的数组元素*/
        for(i=0; i<256; ++i){
            if(ts[i].connfd == -1){
                break;
            }
        }
        if(i == 256){
            break;
        }
		// 主线程 - 等待接受连接请求
        ts[i].connfd = Accept(listenfd, (struct sockaddr*)&ts[i].cliaddr, &cliaddr_len);

        // 创建子线程 - 通信
		pthread_create(&ts[i].pid, NULL, do_work, (void*)&ts[i]);
		
		/*pthread_detach()设置线程分离状态*/
		/*线程分离状态：指定该状态，线程主动与主控线程断开关系。线程结束后（不会产生僵尸线程），
		其退出状态不由其他线程获取，而直接自己自动释放（自己清理掉PCB的残留资源）。
		网络、多线程服务器常用。(注意进程没有这一机制)*/
		pthread_detach(ts[i].pid);
	}
	return 0;
}
