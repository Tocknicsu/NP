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
#include <string>
#include <cstring>
#include <sys/wait.h>
#include <fcntl.h>
class SH{
    struct PIPE{
        int pip[2];
    };
    private:
        int m_count;
        std::map<int, PIPE> pipemap;

        std::vector<std::string> parse_single_cmd(std::string);
        bool find(std::string);
        void external(std::vector<std::string>);
        int internal(std::string);
        std::string get_cwd();
        void create_map_pipe(int);
    public:
        SH();
        int exec(std::string);
        void prompt();
        void init();
        void welcome();
};
#endif
