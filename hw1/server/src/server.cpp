#include "server.h"

SERVER::SERVER(){
}
SERVER::~SERVER(){
    m_log.write("=====Server End=====");
}
void SERVER::init(){
    m_log = LOG("server.log", "server");
    m_config = CONFIG();
    m_log.write("=====Server Start=====");
	if( (m_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        m_log.write("Error: create server socket.");
        exit(1);
	}
    m_log.write("Succ: Server socket create.");
    int m_opt = 1;  //re use addr
    if( setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&m_opt, sizeof(m_opt)) < 0){
        m_log.write("Error: occur at set socket opt.");
        exit(1);
    }
    if( setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEPORT, (char *)&m_opt, sizeof(m_opt)) < 0){
        m_log.write("Error: occur at set socket opt.");
        exit(1);
    }
    m_log.write("Succ: Set socket option.");
	int port = m_config.port();
	int max_wait_listen = m_config.max_wait_listen();
	m_server.sin_family = AF_INET;
	m_server.sin_addr.s_addr = INADDR_ANY;
	m_server.sin_port = htons( port );
	if( bind(m_server_socket, (sockaddr *)&m_server, sizeof(m_server)) < 0){
        m_log.write("Error: occur bind.");
        exit(1);
	}

	if( listen(m_server_socket, max_wait_listen) < 0){
        m_log.write("Error: occur listen.");
        exit(1);
	}

	m_server_len = sizeof(m_server);
    m_log.write("Waiting connect...");

	struct timeval timeout={0, 0};
}
void SERVER::start(){
    while(1){
        int socket_fd = accept(m_server_socket, (sockaddr*)&m_server, (socklen_t*)&m_server_len);
        int pid = fork();
        if(pid){
            clients.insert(pid);
            close(socket_fd);
        } else {
            dup2(socket_fd, STDOUT_FILENO);
            dup2(socket_fd, STDIN_FILENO);
            dup2(socket_fd, STDERR_FILENO);
            std::string str;
            SH sh;
            sh.init();
            sh.welcome();
            while(sh.prompt(), getline(std::cin, str)){
                if(sh.exec(str)==-1)
                    exit(0);
            }
        }
    }
}
