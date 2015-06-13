#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <fstream>
#include "function.h"
class CONFIG{
    private:
        std::string m_ip;
        int m_port;
    public:
        CONFIG();
        ~CONFIG();
        void init();
        std::string ip();
        int port();
};
#endif
