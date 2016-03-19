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
static std::set<int> clients;
class SERVER{
    private:
		sockaddr_in m_server;
		int m_server_socket;
		int m_server_len;

        CONFIG m_config;
        
        LOG m_log;

		fd_set readfd;
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
