#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#include "config.h"
#include "log.h"
#include "function.h"
const int BUFFER_SIZE = 1024;
class CLIENT{
    private:
        LOG m_log;
        CONFIG m_config;

        int m_sock;
        std::string m_server_ip;
        int m_server_port;

        sockaddr_in m_server;

        fd_set readfd;

        std::string nick_name;

        void Set_FD();
        int Command_Line();
        int Client();

    public:
        void Run();

};

#endif
