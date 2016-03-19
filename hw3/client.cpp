#include <bits/stdc++.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
using namespace std;
map<string, string> env;

#define F_CONNECTING 0
#define F_READING 1
#define F_WRITING 2
#define F_PREEXIT 3
#define F_DONE 4

int main(){
    int client_fd[6];
    int client_status[6];
    bool in_use[6] = {0, 0, 0, 0, 0, 1};
    sockaddr_in client_sin[6];
    int conn = 0;
    fd_set rfds, rs, wfds, ws;
    FD_ZERO(&rfds);
    FD_ZERO(&rs);
    FD_ZERO(&wfds);
    FD_ZERO(&ws);
    FILE *file[6];
    string host[6] = {
        "localhost",
        "localhost",
        "localhost",
        "localhost",
        "localhost",
        "localhost"
    };
    string port[6] = {
        "",
        "1738",
        "1738",
        "1738",
        "1738",
        "1738",
    };
    string filename[6] = {
        "",
        "t1.txt",
        "t2.txt",
        "t3.txt",
        "t4.txt",
        "t5.txt"
    };
    for(int i = 1 ; i < 6 ; i++){
        if(in_use[i]){
            client_fd[i] = socket(AF_INET, SOCK_STREAM, 0);
            memset(&client_sin[i], 0, sizeof(client_sin[i]));
            client_sin[i].sin_family = AF_INET;
            client_sin[i].sin_addr = *((in_addr*)gethostbyname(host[i].c_str())->h_addr);
            client_sin[i].sin_port = htons(atoi(port[i].c_str()));
            int flags = fcntl(client_fd[i], F_GETFL, 0);
            fcntl(client_fd[i], F_SETFL, flags | O_NONBLOCK);
            connect(client_fd[i], (sockaddr*)&client_sin[i], sizeof(client_sin[i]));
            client_status[i] = F_CONNECTING;
            conn++;
            FD_SET(client_fd[i], &rs);
            file[i] = fopen(filename[i].c_str(), "r");
        } else {
            client_status[i] = F_DONE;
        }
    }
    int nfds = FD_SETSIZE;
    while (conn > 0) {
        rfds = rs;
        wfds = ws;
        select(nfds, &rfds, &wfds, (fd_set*)0, (struct timeval*)0);
        for(int i = 1 ; i < 6 ; i++){
            if (client_status[i] == F_CONNECTING && (FD_ISSET(client_fd[i], &rfds))){
                client_status[i] = F_READING;
            } else if (client_status[i] == F_WRITING && FD_ISSET(client_fd[i], &wfds)) {
                char buffer[65536] = {};
                fgets(buffer, 65536, file[i]);
                cout << buffer << flush;
                write(client_fd[i], buffer, strlen(buffer));
                if(strncmp(buffer, "exit", 4) == 0){
                    client_status[i] = F_PREEXIT;
                } else {
                    client_status[i] = F_READING;
                }
                FD_SET(client_fd[i], &rs);
                FD_CLR(client_fd[i], &ws);
            } else if (client_status[i] == F_READING && FD_ISSET(client_fd[i], &rfds)) {
                char buffer[65536] = {};
                int n = read(client_fd[i], buffer, 65536);
                cout << buffer << flush;
                for(int j = 0 ; j < n ; j++){
                    if(buffer[j] == '%'){
                        client_status[i] = F_WRITING;
                        FD_SET(client_fd[i], &ws);
                        FD_CLR(client_fd[i], &rs);
                    }
                }
            } else if (client_status[i] == F_PREEXIT && FD_ISSET(client_fd[i], &rfds)) {
                char buffer[65536] = {};
                int n = read(client_fd[i], buffer, 65536);
                cout << buffer << flush;
                FD_CLR(client_fd[i], &rs);
                FD_CLR(client_fd[i], &ws);
                client_status[i] = F_DONE;
                conn--;
            }
        }
    }
    return 0;
}
