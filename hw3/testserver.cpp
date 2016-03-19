#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <csignal>
using namespace std;
set<string> can_exec;

int main(){
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
	int port = 1818;
	int max_wait_listen = 4;
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
    while(1){
        int socket_fd = accept(m_server_socket, (sockaddr*)&m_server, (socklen_t*)&m_server_len);
        if(fork()){
            close(socket_fd);
        } else {
            dup2(socket_fd, STDIN_FILENO);
            dup2(socket_fd, STDOUT_FILENO);
            close(socket_fd);
            string str;
            cout << "%" << fflush;
            while(getline(cin, str)){
                cout << "[%";
                cerr << "[%";
                for(int i = 0 ; i < (int)str.size() ; i++)
                    if(str[i] != '\r'){
                        cout << str[i];
                        cerr << str[i];
                    }
                cerr << "]" << endl << fflush;
                cout << "]" << endl;
            }
            exit(0);
        }
    }
    return 0;
}
