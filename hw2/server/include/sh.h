#ifndef SH_H
#define SH_H
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/wait.h>
#include <fcntl.h>
#include <set>
#include <list>
#include "config.h"
class SH{
    struct PIPE{
        int pip[2];
    };
    public:
        int id;
        SHARE_MEMORY *msgptr;
    private:
        int m_count;
        int KEY;
        std::map<int, PIPE> pipemap;


        std::vector<std::string> parse_single_cmd(std::string);
        bool find(std::string);
        void external(std::vector<std::string>);
        int internal(std::string);
        std::string get_cwd();
        void create_map_pipe(int);

        void BoardCast(std::string);

        void who();
        void name(std::string);

    public:
        SH();
        int exec(std::string);
        void Exit();
        void clear_buffer();
        void Sent_Message(int, std::string);
        void prompt();
        void init(int);
        void welcome();
        void change_dir();
        void recover_dir();
};
#endif
