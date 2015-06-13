#ifndef CLIENT_H
#define CLIENT_H
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
using namespace std;

class CLIENT{
	private:
		int m_client_socket;
		sockaddr_in m_addr;
		socklen_t m_addr_len;
		char m_ip[32];
		int m_status;
		int m_verify_code;
		string m_version;
	public:
		CLIENT();
		void set_socket(int);
};
#endif
