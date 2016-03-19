#include "server.h"
#include <unistd.h>
#include <csignal>
#include <sys/wait.h>

int main(){
    signal(SIGCHLD, SIG_IGN); 
    SERVER server;
    server.init();
    server.start();
}
