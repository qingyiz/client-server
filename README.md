# client-server
### c/s各个版本
### 普通版本
    gcc server.c wrap.c -o server
    gcc client.c wrap.c -o client
    ./server xxxx(端口号)
    ./client xxxx(端口号)
### 多进程并发
    gcc process_server.c wrap.c -o process_server
    gcc process_client.c wrap.c -o process_client
    ./process_server xxxx(端口号)
    ./process_client xxxx(端口号)
### 多线程并发
    gcc pthread_server.c wrap.c -o pthread_server -lpthread
    gcc pthread_client.c wrap.c -o pthread_client
    ./pthread_server xxxx(端口号)
    ./pthread_client xxxx(端口号)

### 多路IO转接
    select_mode:
      gcc select.c wrap.c -o select
      gcc client.c wrap.c -o client
      ./select xxxx(端口号)
      ./client xxxx(端口号)
    poll_mode:
      gcc poll.c wrap.c -o poll
      gcc client.c wrap.c -o client
      ./poll xxxx(端口号)
      ./client xxxx(端口号)
    
    
#### 客户端代码都是一样的。
