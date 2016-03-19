#ifndef SERVER_H
#define SERVER_H
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <list>
#include <cstring>
#include <set>
#include "log.h"
#include "config.h"
#include "function.h"
#include "sh.h"
class SERVER{
    private:
        std::set<int> ID_POOL;
		sockaddr_in m_server;
		int m_server_socket;
		int m_server_len;

        CONFIG m_config;

        SH m_client_list[32];
        std::map<int, PIPE> global_pipe;
        //PIPE global_pipe[128];

        fd_set readfd;
        
        LOG m_log;

        void Server();
        void Client();

        void Set_FD();

        void change_fd(int);
        void recover_fd();

        int server_stdin;
        int server_stdout;
        int server_stderr;


    public:
        SERVER();
        ~SERVER();
        void init();
        void start();
        void stop();
};
#endif
