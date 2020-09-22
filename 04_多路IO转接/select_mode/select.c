#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "wrap.h"

int main(int argc, const char* argv[])
{
    if(argc < 2){
        printf("eg: ./a.out port\n");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    socklen_t serv_len = sizeof(serv_addr);
    int port = atoi(argv[1]);

    /*创建套接字*/
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    /* 初始化服务器 sockaddr_in */
    memset(&serv_addr, 0, serv_len);
    serv_addr.sin_family = AF_INET;                   	/* 地址族 */
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);    	/* 监听本机所有的IP */
    serv_addr.sin_port = htons(port);            		/* 设置端口 */
    /* 绑定IP和端口 */
    Bind(listenfd, (struct sockaddr*)&serv_addr, serv_len);

    /*设置端口复用*/
    
      int opt = 1; 
      setsockopt(listenfd,
                 SOL_SOCKET, 
                 SO_REUSEPORT,
                 (const void *)&opt,
                 sizeof(opt));
    


    /* 设置同时监听的最大个数 */
    Listen(listenfd, 36);
    printf("Start accept ......\n");

    struct sockaddr_in client_addr;
    socklen_t cli_len = sizeof(client_addr);

    /* 最大的文件描述符 */
    int maxfd = listenfd;
    /* 文件描述符读集合*/
    fd_set reads, temp; /*原始表和被修改表*/
    /* init	*/
    FD_ZERO(&reads);
    FD_SET(listenfd, &reads);

    while(1)
    {
        /* 委托内核做IO检测*/
        temp = reads;
        int ret = select(maxfd+1, &temp, NULL, NULL, NULL);
        if(ret == -1){
            perror("select error");
            exit(1);
        }
        /* 客户端发起了新的连接*/
        if(FD_ISSET(listenfd, &temp)){
            /* 接受连接请求 - accept不阻塞*/
            int cfd = Accept(listenfd, (struct sockaddr*)&client_addr, &cli_len);
            char ip[64];
            printf("new client IP: %s, Port: %d\n", 
                   inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)),
                   ntohs(client_addr.sin_port));
            /* 将cfd加入到待检测的读集合中 - 下一次就可以检测到了*/
            FD_SET(cfd, &reads);
            /* 更新最大的文件描述符*/
            maxfd = maxfd < cfd ? cfd : maxfd;
        }
        /* 已经连接的客户端有数据到达 */
        for(int i=listenfd+1; i<=maxfd; ++i){
            if(FD_ISSET(i, &temp)){
                char buf[1024] = {0};
                int len = recv(i, buf, sizeof(buf), 0);
                if(len == -1){
                    perror("recv error");
                    exit(1);
                }
                else if(len == 0){
                    printf("客户端已经断开了连接\n");
                    close(i);
                    /* 从读集合中删除 */
                    FD_CLR(i, &reads);
                }
                else{
                    printf("recv buf: %s\n", buf);
                    int n = strlen(buf);
                    for(int i = 0;i < n;i++){
                        buf[i] = toupper(buf[i]);

                    }
                    send(i, buf, n+1, 0);
                }
            }
        }
    }

    close(listenfd);
    return 0;
}

