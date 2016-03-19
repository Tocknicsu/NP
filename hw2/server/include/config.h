#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <fstream>
#include <iostream>
#include <set>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#define BIGCOUNT 100
#include "function.h"
#define __LOG 1
#define BUFFER_SIZE 16384
class CONFIG{
    private:
        int m_port;
        int m_max_wait_listen;
    public:
        CONFIG();
        ~CONFIG();
        void init();
        int port();
        int max_wait_listen();
};

struct MSG{
    int num;
    char text[256][1025];
};

struct CLIENT{
    bool        in_use;
    int         socket_fd;
    sockaddr_in addr;
    socklen_t   addr_len;
    char        ip[32];
    int         port;
    char        nick_name[32];
    MSG         msg;
};
struct SHARE_MEMORY{
    CLIENT  clients[32];
    int     global_pipe_num[128];
    bool    global_pipe[128];
};
int sem_create(key_t, int);
int sem_open(key_t);
void sem_wait(int);
void sem_signal(int);
void sem_op(int, int);
void sem_close(int);
void sem_rm(int);
#endif
