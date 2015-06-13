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
#include "log.h"
#include "config.h"
#include "function.h"

const int BUFFER_SIZE = 1024;
class SERVER{
    struct CLIENT{
        int         socket;
        sockaddr_in addr;
        socklen_t   addr_len;
        std::string ip;
        std::string nick_name;
    };
    private:
		sockaddr_in m_server;
		int m_server_socket;
		int m_server_len;

        CONFIG m_config;
        
        std::list<CLIENT> m_client_list;

        LOG m_log;

		fd_set readfd;
        
        void Set_FD();
        void Server();
        void Client();
        int Command_Line();
    public:
        SERVER();
        ~SERVER();
        void start();
        void stop();
};

#endif
/*
#ifndef SERVER_H
#define SERVER_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <iostream>
using namespace std;
#include "config.h"
#include "client.h"
#include "class_mysql.h"
#define BUFFER 1024


class SERVER{
	private:
		CONFIG m_config;
		int m_server_socket;
		int m_server_len;

		list<int> m_submission_list;
		list<CLIENT> m_client_list;

		sockaddr_in m_server;

		fd_set readfd;

		bool sent_submission;
	public:
		SERVER();
		~SERVER();
		
		void init();
		void run();

		int Set_FD();	//return max code of fd
		int Command_Line();
		void Server();
		void Client();	

		void Get_Submission();
		void Sent_Submission();
};


#endif
*/
