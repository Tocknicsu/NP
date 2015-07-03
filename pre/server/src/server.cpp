#include "server.h"

SERVER::SERVER(){
}
SERVER::~SERVER(){
    m_log.write("=====Server End=====");
}
void SERVER::start(){
    m_log = LOG("server.log", "server");
    m_config = CONFIG();

    m_log.write("=====Server Start=====");

	if( (m_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        m_log.write("Error: create server socket.");
        std::cerr << "Error: create server socket." << std::endl;
	    return;
	}
    m_log.write("Succ: Server socket create.");
    std::cout << "Succ: Server socket create." << std::endl;

    int m_opt = 1;  //re use addr
    if( setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&m_opt, sizeof(m_opt)) < 0){
        m_log.write("Error: occur at set socket opt.");
        return;
    }
    m_log.write("Succ: Set socket option.");

	int port = m_config.port();
	int max_wait_listen = m_config.max_wait_listen();

	m_server.sin_family = AF_INET;
	m_server.sin_addr.s_addr = INADDR_ANY;
	m_server.sin_port = htons( port );

	if( bind(m_server_socket, (sockaddr *)&m_server, sizeof(m_server)) < 0){
		return;
	}

	if( listen(m_server_socket, max_wait_listen) < 0){
		return;
	}

	m_server_len = sizeof(m_server);
    std::cout << "Waiting connect..." << std::endl;
	int submission_update = 0;

	struct timeval timeout={0, 0};

    std::cout << m_server_socket << std::endl;
    m_server_len = sizeof(m_server);
	while(true){
        Set_FD();

        int max_sd = m_server_socket;
        for(auto it : m_client_list)
            max_sd = max_sd > it.socket ? max_sd : it.socket;

        int result = select(max_sd + 1, &readfd, NULL, NULL, &timeout);
        if(result == 0){
            usleep(1000);
            continue;
        }
        {
            int com_result = Command_Line();
            if(com_result == -1)
                return;
        }
        {
            Server();
        }
        {
            Client();
        }
	}
}

void SERVER::Set_FD(){
    FD_ZERO(&readfd);
    FD_SET(0, &readfd); //stdin
    FD_SET(m_server_socket, &readfd);
    for(auto it : m_client_list)
        FD_SET(it.socket, &readfd);
}
int SERVER::Command_Line(){
    if(FD_ISSET(0, &readfd)){
        int result;
        char buffer[BUFFER_SIZE+1];
        if( (result = read(0, buffer, BUFFER_SIZE)) ){
            buffer[result-1] = '\0';
            std::stringstream ss(buffer);
            std::string command;
            ss >> command;

            std::cout << "Input: " << command << std::endl;
            m_log.write("[Stdin]" + command);
            if(command == std::string("exit")){
                return -1;
            }
        }
    }
    return 0;
}
void SERVER::Server(){
    if(FD_ISSET(m_server_socket, &readfd)){
        CLIENT new_client;
        new_client.socket = accept(m_server_socket, (sockaddr*)&m_server, (socklen_t*)&m_server_len);
        new_client.addr_len = sizeof(new_client.addr);
        getpeername(new_client.socket, (sockaddr*)&new_client.addr, &new_client.addr_len);
        char t_ip[32];
        inet_ntop(AF_INET, &new_client.addr.sin_addr, t_ip, sizeof(t_ip));
        new_client.ip = std::string(t_ip);
        m_client_list.push_back(new_client);
        std::cout << "New Connection: " << new_client.ip << std::endl;
        m_log.write(std::string("Connected") + new_client.ip);
    }
}
void SERVER::Client(){
    int result;
    char buffer[BUFFER_SIZE+1], msg[BUFFER_SIZE+1];
    for(std::list<CLIENT>::iterator it = m_client_list.begin() ; it != m_client_list.end() ;){
        if(FD_ISSET(it->socket, &readfd)){
            int result = read(it->socket, buffer, BUFFER_SIZE);
            buffer[result] = 0;
            if( result == 0){
                sprintf(msg, "[System] %s Exited.\r\n", it->nick_name.c_str());
                std::cout << "Close Connection: " << it->ip << std::endl;
                std::cout << msg;
                m_log.write(std::string("Closed ") + it->ip);
                std::list<CLIENT>::iterator tmp = it;
                it = ++it;
                m_client_list.erase(tmp);

                for(auto client : m_client_list)
                    write(client.socket, msg, strlen(msg));
                continue;
            } else {
                std::stringstream ss(buffer);
                std::string str;
                ss >> str;
                if(str == "/init_nick"){
                    ss >> it->nick_name;
                    sprintf(msg, "[System] %s Join.\r\n", it->nick_name.c_str());
                    {
                        char hello_msg[BUFFER_SIZE+1];
                        sprintf(hello_msg, "[System] Hello %s\r\n", it->nick_name.c_str());
                        write(it->socket, hello_msg, strlen(hello_msg));
                    }
                } else if(str == "/nick"){
                    std::string old_nick_name = it->nick_name;
                    ss >> it->nick_name;
                    sprintf(msg, "[System] [%s] change to [%s]\r\n", old_nick_name.c_str(), it->nick_name.c_str());
                } else {
                    sprintf(msg, "[%s]: %s\r\n", it->nick_name.c_str(), buffer);
                }
                for(auto client : m_client_list)
                    write(client.socket, msg, strlen(msg));
                std::cout << msg;
            }
        }
        ++it;
    }
}

void SERVER::stop(){

}
