#include "server.h"
#include <unistd.h>
#include <csignal>
#include <sys/wait.h>

void signal_handler_SIGCHLD(int sig){
    int status;
    int pid;
    while(pid = waitpid(-getpid(), &status, WNOHANG), pid!=-1);
}

int main(){
    signal(SIGCHLD, &signal_handler_SIGCHLD); 
    SERVER server;
    server.init();
    server.start();
    for(auto x : clients){
        kill(x, SIGINT);
        waitpid(x, NULL, 0);
    }
}
