#include "function.h"


std::string exec_path(){
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string(result, std::max(int(count), 0));
}
std::string root_dir(){
    std::string re = exec_path();
    re = re.substr(0, re.find_last_of("/"));
    re = re.substr(0, re.find_last_of("/")+1);
    return re;
}
