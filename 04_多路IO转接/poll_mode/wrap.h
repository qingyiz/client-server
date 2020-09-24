#ifndef __WRAP_H_
#define __WRAP_H_
#include <sys/socket.h>

void perr_exit(const char *s);

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);		/*accept()*/

int Bind(int fd, const struct sockaddr *sa, socklen_t salen);		/*bind()*/

int Connect(int fd, const struct sockaddr *sa, socklen_t salen);	/*connect()*/

int Listen(int fd, int backlog);									/*listen()*/

int Socket(int family, int type, int protocol);						/*socket()*/

ssize_t Read(int fd, void *ptr, size_t nbytes);						/*read()*/

ssize_t Write(int fd, const void *ptr, size_t nbytes);				/*write()*/

int Close(int fd);													/*close()*/

ssize_t Readn(int fd, void *vptr, size_t n);						/**/

ssize_t Writen(int fd, const void *vptr, size_t n);					/**/

ssize_t my_read(int fd, char *ptr);									/**/

ssize_t Readline(int fd, void *vptr, size_t maxlen);				/**/
#endif
