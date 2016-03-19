#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <csignal>
using namespace std;
set<string> can_exec;

int main(){
    can_exec.insert("cgi");
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
	int port = 1780;
	int max_wait_listen = 4;
	cout << "Port: " << port << endl;
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
            string query_type, url, file_path, query_string, file_extension;
            cin >> query_type >> url;
            if(url.size() > 0)
                file_path = url.substr(1, url.find_last_of('?')-1);
            if(file_path.find(".") != string::npos)
                file_extension = file_path.substr(file_path.find_last_of('.') + 1, file_path.size() - file_path.find_last_of('.'));
            if(url.find("?") != string::npos)
                query_string = url.substr(url.find_last_of('?') + 1, url.size() - url.find_last_of('?'));
            while(query_string.find("%2F") != string::npos){
                query_string.replace(query_string.find("%2F"), 3, "/");
            }
            

            cout << "HTTP/1.1 200 OK" << endl;
            if(can_exec.count(file_extension)){
                clearenv();
                setenv("QUERY_STRING", query_string.c_str(), 0);
                setenv("SCRIPT_NAME", file_path.c_str(), 0);
                setenv("REQUEST_METHOD", query_type.c_str(), 0);
                setenv("REMOTE_ADDR", "127.0.0.1", 0);
                setenv("REMOTE_HOST", "localhost", 0);
                setenv("CONTENT_LENGTH", "1024", 0);
                setenv("AUTH_TYPE", "None", 0);
                setenv("REMOTE_USER", "www", 0);
                setenv("REMOTE_IDENT", "www", 0);
                execl(file_path.c_str(), file_path.c_str(), NULL);
                exit(0);
            } else {
                cout << "Content-type: text/html" << endl << endl;
                ifstream f(file_path.c_str());
                string str;
                while(getline(f, str)){
                    cout << str << endl;
                }
            }
            exit(0);
        }
    }
    return 0;
}
