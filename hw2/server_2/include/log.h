#ifndef LOG_H
#define LOG_H

#include <string>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include "config.h"


class LOG{
    private:
        std::string m_file_path;
        std::string m_service_name;
    public:
        LOG();
        LOG(std::string);   //set path
        LOG(std::string, std::string);   //set path
        void write(std::string);
};

#endif
