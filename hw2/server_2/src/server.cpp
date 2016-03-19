#include "server.h"

SERVER::SERVER(){
    for(int i = 1 ; i <= 30 ; i++)
        ID_POOL.insert(i);
    for(int i = 0 ; i < 128 ; i++)
        global_pipe[i].in_use = false;
}
SERVER::~SERVER(){
    m_log.write("=====Server End=====");
}
void SERVER::init(){
    server_stdin = dup(STDIN_FILENO);
    server_stdout = dup(STDOUT_FILENO);
    server_stderr = dup(STDERR_FILENO);
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
    while(true){
        m_log.write("Set FD");
        Set_FD();
        int max_fd = m_server_socket;
        for(auto it : m_client_list)
            max_fd = std::max(max_fd, it.socket_fd);

        int result = select(max_fd + 1, &readfd, NULL, NULL, NULL);

        Server();
        Client();
        
    }
}
void SERVER::Set_FD(){
    FD_ZERO(&readfd);
    FD_SET(0, &readfd);                 //stdin
    FD_SET(m_server_socket, &readfd);   //server
    //clients
    for(int i = 1 ; i <= 30 ; i++){
        if(m_client_list[i].in_use){
            FD_SET(m_client_list[i].socket_fd, &readfd);
        }
    }
}

void SERVER::change_fd(int fd){
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
}
void SERVER::recover_fd(){
    dup2(server_stdin, STDIN_FILENO);
    dup2(server_stdout, STDOUT_FILENO);
    dup2(server_stderr, STDERR_FILENO);
}
void SERVER::Server(){
    if(FD_ISSET(m_server_socket, &readfd)){
        int new_id = *ID_POOL.begin();
        ID_POOL.erase(new_id);
        m_client_list[new_id].in_use = true;
        m_client_list[new_id].id = new_id;
        m_client_list[new_id].socket_fd = accept(m_server_socket, (sockaddr*)&m_server, (socklen_t*)&m_server_len);
        m_client_list[new_id].addr_len = sizeof(m_client_list[new_id].addr);
        getpeername(m_client_list[new_id].socket_fd, (sockaddr*)&m_client_list[new_id].addr, &m_client_list[new_id].addr_len);
        char t_ip[32];
        inet_ntop(AF_INET, &m_client_list[new_id].addr.sin_addr, t_ip, sizeof(t_ip));  // get ip
        m_client_list[new_id].ip = std::string(t_ip);                                  // set ip
        m_client_list[new_id].ip = std::string("CGILAB");
        m_client_list[new_id].port = ntohs(m_client_list[new_id].addr.sin_port);                  // set port
        m_client_list[new_id].port = 511;

        m_client_list[new_id].clients = m_client_list;                                // set clients
        m_client_list[new_id].global_pipe = &global_pipe;                                //set global pipe
        /* print welcome information */
        change_fd(m_client_list[new_id].socket_fd);
        m_client_list[new_id].init();
        m_client_list[new_id].welcome();
        m_client_list[new_id].prompt();
        recover_fd();

        m_log.write("New Connection:" + std::string(m_client_list[new_id].ip));
    }
}

void SERVER::Client(){
    int result;
    for(int i = 1 ; i <= 30 ; i++){
        if(m_client_list[i].in_use == false) continue;
        if(FD_ISSET(m_client_list[i].socket_fd, &readfd)){
            bool end = false;
            char str[65536] = {};
            int re = read(m_client_list[i].socket_fd, str, 65536);
            if(re > 0){
                m_log.write("From:[" + std::to_string(m_client_list[i].socket_fd) + "] Command: " + str);
                change_fd(m_client_list[i].socket_fd);
                m_client_list[i].change_dir();
            
                if(m_client_list[i].exec(str) == -1){
                    end = true;
                } else {
                    m_client_list[i].prompt();
                }
                m_client_list[i].recover_dir();
                recover_fd();
            } else if (re == 0){
                m_client_list[i].Exit();
                end = true;
            }
            if(end){
                ID_POOL.insert(m_client_list[i].id);
                close(m_client_list[i].socket_fd);
                m_client_list[i].in_use = false;
            }
        }
    }
}
