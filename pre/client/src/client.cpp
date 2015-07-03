#include "client.h"


void CLIENT::Run(){
    std::cout << "Enter your nick name: ";
    std::cin >> nick_name;
    m_log = LOG("client.log");

    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sock == -1){
        std::cout << "QQ" << std::endl;
        return;
    }

    m_server.sin_addr.s_addr = inet_addr(m_config.ip().c_str());
    m_server.sin_family = AF_INET;
    m_server.sin_port = htons(m_config.port());

    if( connect(m_sock, (sockaddr*)&m_server, sizeof(m_server)) < 0){
        return;
    }
    {
        char buffer[BUFFER_SIZE+1];
//        sprintf(buffer, "/init_nick %s\r\nXDDD\r\nQQQQ\r\n", nick_name.c_str());
//        write(m_sock, buffer, strlen(buffer));
    }
    struct timeval timeout = {0, 0};
    while(true){
        Set_FD();
        int result = select(m_sock+1, &readfd, 0, 0, &timeout);
        if(result == 0){
            usleep(1000);
        }
        {
            int result = Command_Line();
            if(result == -1)
                return;
        }
        {
            int result = Client();
            if(result == -1)
                return;
        }
    }
}

void CLIENT::Set_FD(){
    FD_ZERO(&readfd);
    FD_SET(0, &readfd);
    FD_SET(m_sock, &readfd);
}


int CLIENT::Command_Line(){
    if(FD_ISSET(0, &readfd)){
        char buffer[BUFFER_SIZE+1];
        int result = read(0, buffer, BUFFER_SIZE);
        buffer[result-1] = 0;
        std::stringstream ss(buffer);
        std::string str;
        ss >> str;
        if(str == std::string("exit")){
            return -1;
        } else {
            write(m_sock, buffer, strlen(buffer));
        }
    }
    return 0;
}
int CLIENT::Client(){
    if(FD_ISSET(m_sock, &readfd)){
        char buffer[BUFFER_SIZE+1];
        int result = read(m_sock, buffer, BUFFER_SIZE);
        buffer[result] = 0;
        if(result == 0){
            std::cout << "Server closed." << std::endl;
            return -1;
        } else {
            std::cout << buffer;
        }
    }
    return 0;
}
