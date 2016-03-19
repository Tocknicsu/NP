#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <csignal>
#include <netdb.h>
#include <fcntl.h>
using namespace std;
set<string> can_exec;

void handle(int, sockaddr_in);
void transmit(int,int);

int main(){
	printf("Server start.\n");
    signal(SIGCHLD, SIG_IGN); 
    sockaddr_in m_server;
    int m_server_socket;
    int m_server_len;
	if( (m_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        exit(1);
	}
    int m_opt = 1;  //re use addr
    if( setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&m_opt, sizeof(m_opt)) < 0){
        exit(1);
    }
    if( setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEPORT, (char *)&m_opt, sizeof(m_opt)) < 0){
        exit(1);
    }
	int port = 8880;
	int max_wait_listen = 4;
	printf("Port: %d\n", port);
	m_server.sin_family = AF_INET;
	m_server.sin_addr.s_addr = INADDR_ANY;
	m_server.sin_port = htons( port );
	if( bind(m_server_socket, (sockaddr *)&m_server, sizeof(m_server)) < 0){
        exit(1);
	}

	if( listen(m_server_socket, max_wait_listen) < 0){
        exit(1);
	}
	m_server_len = sizeof(m_server);
	sockaddr_in m_client;
	int m_client_len = sizeof(m_client);
    while(1){
        int socket_fd = accept(m_server_socket, (sockaddr*)&m_client, (socklen_t*)&m_client_len);
        if(fork()){
            close(socket_fd);
        } else {
			handle(socket_fd, m_client);
            exit(0);
        }
    }
    return 0;
}

void handle(int client_fd, sockaddr_in client){
	printf("=============\n");
	printf("Source=%s(%u)\n", inet_ntoa(client.sin_addr), client.sin_port);
	char buffer[256];
	read(client_fd, buffer, sizeof(buffer));
	unsigned char VN = buffer[0];
	unsigned char CD = buffer[1];
	unsigned int DST_PORT = (unsigned char)(buffer[2]) << 8  |
							(unsigned char)(buffer[3]);
	unsigned int DST_IP =   (unsigned char)(buffer[7]) << 24 |
							(unsigned char)(buffer[6]) << 16 | 
							(unsigned char)(buffer[5]) << 8  | 
							(unsigned char)(buffer[4]);
	printf("VN=%u CD=%u(%s)\n", VN, CD, (CD==0x01)?"CONNECT":(CD==0x02)?"BIND":"Known");
	printf("Destination=%u.%u.%u.%u(%u)\n", (unsigned char)(buffer[4]), (unsigned char)buffer[5], (unsigned char)buffer[6], (unsigned char)buffer[7], DST_PORT);
	if(VN != 0x04)
		return;
	buffer[0] = 0;
	/* firewall */
	buffer[1] = 0x5B;
	string str;
	ifstream ifs("./firewall.conf");
	while(getline(ifs, str)){
		unsigned int dst_ip =   (unsigned char)(buffer[4]) << 24 |
								(unsigned char)(buffer[5]) << 16 | 
								(unsigned char)(buffer[6]) << 8  | 
								(unsigned char)(buffer[7]);
		printf("Checking firewall rule: %s\n", str.c_str());
		unsigned int ip[4], mask;
		sscanf(str.c_str(), "%d.%d.%d.%d/%d", &ip[0], &ip[1], &ip[2], &ip[3], &mask);
		unsigned int rule_ip =	ip[0] << 24 |
								ip[1] << 16 |
								ip[2] << 8	|
								ip[3];
		if((((long long)rule_ip) >> (32-mask)) == (((long long)dst_ip) >> (32-mask))){
			printf("Pass firewall by rule: %s\n", str.c_str());
			buffer[1] = 0x5A;
			break;
		}
	}
	if(buffer[1] == 0x5B){
		printf("Cannot pass any firewall rule.\n");
	}
	int remote_fd;
	sockaddr_in remote;
	if(CD == 0x01){
		write(client_fd, buffer, 8);
		remote_fd = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in remote;
		remote.sin_family = AF_INET;
		remote.sin_addr.s_addr = DST_IP;
		remote.sin_port = htons(DST_PORT);
		if(connect(remote_fd, (sockaddr*)&remote, sizeof(remote)) == -1){
			printf("Failed\n");
			return;
		}
	} else if(CD==0x02){
		int bind_fd = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in bind_add;
		bind_add.sin_family = AF_INET;
		bind_add.sin_addr.s_addr = htonl(INADDR_ANY);
		bind_add.sin_port = htons(INADDR_ANY);
		int m_opt = 1;
		if( setsockopt(bind_fd, SOL_SOCKET, SO_REUSEPORT, (char *)&m_opt, sizeof(m_opt)) < 0){
			printf("set sock error\n");
		}

		if(bind(bind_fd, (sockaddr*)&bind_add, sizeof(bind_add)) < 0){
			printf("Bind Failed.\n");
		}
		sockaddr_in get_port;
		int get_port_len = sizeof(get_port);
		if(getsockname(bind_fd, (sockaddr*)&get_port, (socklen_t*)&get_port_len)<0){
			printf("Get error\n");
		}
		if(listen(bind_fd, 4)<0){
			printf("Listen error\n");
		}
		buffer[2] = (unsigned char)(ntohs(get_port.sin_port)/256);
		buffer[3] = (unsigned char)(ntohs(get_port.sin_port)%256);
		buffer[4] = 0;
		buffer[5] = 0;
		buffer[6] = 0;
		buffer[7] = 0;
		write(client_fd, buffer, 8);
		int remote_len = sizeof(remote);
		remote_fd = accept(bind_fd, (sockaddr*)&remote, (socklen_t*)&remote_len);
		write(client_fd, buffer, 8);
	} else {
		printf("Unknown\n");
		exit(0);
	}
	transmit(client_fd, remote_fd);

}
void transmit(int client_fd, int remote_fd){
	int nfds = max(remote_fd, client_fd) + 1;
	fd_set rfds, rs;
	FD_ZERO(&rs);
	FD_SET(remote_fd, &rs);
	FD_SET(client_fd, &rs);
	char buffer[65536];
	while(1){
		rfds = rs;
		select(nfds, &rfds, NULL, NULL, (struct timeval*)0);
		int ret;
		if(FD_ISSET(remote_fd, &rfds)){
			ret =read(remote_fd, buffer, sizeof(buffer));
			if(ret == 0){
				exit(0);
			} else if(ret == -1){
				exit(0);
			} else {
				for(int i = 0 ; i < min(10, ret) ; i++)
					printf("%3x", (unsigned char)buffer[i]);
				printf("\n");
				ret = write(client_fd, buffer, ret);
			}
		}
		if(FD_ISSET(client_fd, &rfds)){
			ret =read(client_fd, buffer, sizeof(buffer));
			if(ret == 0){
				exit(0);
			} else if(ret == -1){
				exit(0);
			} else {
				for(int i = 0 ; i < min(10, ret) ; i++)
					printf("%3x", (unsigned char)buffer[i]);
				printf("\n");
				ret = write(remote_fd, buffer, ret);
			}
		}
	}
}
