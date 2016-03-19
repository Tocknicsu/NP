#ifndef SH_H
#define SH_H
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sstream>
#include <map>
#include <vector>
#include <iostream>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <sys/wait.h>
#include <fcntl.h>
#include <set>
#include <list>
struct PIPE{
    int pip[2];
    bool in_use;
};
class SH{
    public:
        bool        in_use;
        int         id;
        int         socket_fd;
        sockaddr_in addr;
        socklen_t   addr_len;
        std::string ip;
        int         port;
        std::string nick_name;
        SH *clients;
        std::map<int, PIPE> *global_pipe;
    private:
        int m_count;
        std::map<int, PIPE> pipemap;

        std::string PATH;

        std::vector<std::string> parse_single_cmd(std::string);
        bool find(std::string);
        void external(std::vector<std::string>);
        int internal(std::string);
        std::string get_cwd();
        void create_map_pipe(int);

        void BoardCast(std::string);

        void who();
        void name(std::string);

        void change_fd(int);
    public:
        SH();
        int exec(std::string);
        void Exit();
        void prompt();
        void init();
        void welcome();
        void change_dir();
        void recover_dir();
};
#endif
