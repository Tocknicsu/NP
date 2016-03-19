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
}
void SERVER::start(){
    SHARE_MEMORY *msgptr;
    int shmid;
    int SHMKEY = 123;
    int KEY;
    // server is 31 
    if( (shmid = shmget(SHMKEY, sizeof(SHARE_MEMORY), 0666 | IPC_CREAT)) < 0){
        m_log.write("Create Share Memory Failed.");
        exit(1);
    }
    if( (msgptr = (SHARE_MEMORY*)shmat(shmid, (char*)0, 0)) == (SHARE_MEMORY*)-1){
        m_log.write("Get Share Memory Failed.");
    }
    if( (KEY = sem_create(key_t(1000), 1)) < 0 ){
        m_log.write("Create Key Failed");
    }
    for(int i = 1 ; i <= 30 ; i++){
        msgptr->clients[i].in_use = false;
        memset(msgptr->clients[i].msg.text, 0, sizeof(msgptr->clients[i].msg.text));
        msgptr->clients[i].msg.num = 0;
    }
    for(int i = 0 ; i < 128 ; i++){
        msgptr->global_pipe[i] = 0;
    }

    int time = 0;
    int flags = fcntl(m_server_socket, F_GETFL, 0);
    fcntl(m_server_socket, F_SETFL, flags | O_NONBLOCK);
    while(1){
        usleep(100000);
        int socket_fd = accept(m_server_socket, (sockaddr*)&m_server, (socklen_t*)&m_server_len);
        if(socket_fd != -1){
            sem_wait(KEY);
            int new_id;
            for(int i = 1 ; i <= 30 ; i++){
                if(msgptr->clients[i].in_use == false){
                    new_id = i;
                    break;
                }
            }
            std::cout << new_id << std::endl;
            char t_ip[32];
            getpeername(socket_fd, (sockaddr*)&msgptr->clients[new_id].addr, &msgptr->clients[new_id].addr_len);
            inet_ntop(AF_INET, &msgptr->clients[new_id].addr.sin_addr, t_ip, sizeof(t_ip));  // get ip
            strcpy(msgptr->clients[new_id].ip, t_ip);                                  // set ip
            strcpy(msgptr->clients[new_id].ip, "CGILAB");
            msgptr->clients[new_id].port = ntohs(msgptr->clients[new_id].addr.sin_port);                  // set port
            msgptr->clients[new_id].port = 511;
            msgptr->clients[new_id].in_use = true;
            msgptr->clients[new_id].socket_fd = socket_fd;
            strcpy(msgptr->clients[new_id].nick_name, "(no name)");


            int pid = fork();
            if(pid == 0){
                int flags = fcntl(socket_fd, F_GETFL, 0);
                fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
                dup2(socket_fd, STDIN_FILENO);
                dup2(socket_fd, STDOUT_FILENO);
                dup2(socket_fd, STDERR_FILENO);
                close(socket_fd);

                char str[65536];
                SH sh;
                sh.id = new_id;
                sh.msgptr = msgptr;
                sh.init(KEY);
                sh.welcome();
                sh.prompt();
				std::string buffer;
                while(true){
                    usleep(100000);
                    memset(str, 0, sizeof(str));
                    sh.clear_buffer();
                    int len;
                    if(len = read(STDIN_FILENO, str, 65536), len > 0){
						bool go_to_shell_flag = false;
						for(int i = 0 ; i < len ; i++){
							if(str[i] == '\n'){
								go_to_shell_flag = true;
							}
						}
						buffer += std::string(str);
						if(!go_to_shell_flag) continue;
                        int result = sh.exec(buffer);
						buffer.clear();
                        sh.clear_buffer();
                        if(result==-1){
                            msgptr->clients[sh.id].in_use = false;
                            shmdt(msgptr);
                            exit(0);
                        }
                        sh.prompt();
                    }
                    if(len == 0){
                        msgptr->clients[sh.id].in_use = false;
                        sh.Exit();
                        shmdt(msgptr);
                        exit(0);
                    }
                }
            } else if (pid > 0) {
                close(socket_fd);
            } else {
                std::cout << "Fork gg" << std::endl;
            }
            sem_signal(KEY);
        }
    }
    if( shmdt(msgptr) < 0 ){
        m_log.write("Detach Shared Memory Failed.");
    }
    shmdt(msgptr);
}

