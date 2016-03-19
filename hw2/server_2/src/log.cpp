#include "log.h"

LOG::LOG(){
}

LOG::LOG(std::string _file_path){
    m_file_path = _file_path;
}

LOG::LOG(std::string _file_path, std::string _service_name){
    m_file_path = _file_path;
    m_service_name = _service_name;
}

void LOG::write(std::string msg){
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
#if __LOG == 1
    std::ofstream ofs(m_file_path, std::ios::app);
    if(ofs.is_open()){
        ofs << "[" 
            << timeinfo->tm_year + 1990 << "/"
            << std::setfill('0') << std::setw(2) << timeinfo->tm_mon + 1 << "/"
            << std::setfill('0') << std::setw(2) << timeinfo->tm_mday << " "
            << std::setfill('0') << std::setw(2) << timeinfo->tm_hour << ":"
            << std::setfill('0') << std::setw(2) << timeinfo->tm_min << ":"
            << std::setfill('0') << std::setw(2) << timeinfo->tm_sec
            << "] ";
        ofs  << "[" << getpid() << "] ";
        ofs << msg << std::endl;
        ofs.close();
    } else {
        std::cout << "write log error" << std::endl;
    }
#endif
    /*
    std::cout << "[" 
        << timeinfo->tm_year + 1990 << "/"
        << std::setfill('0') << std::setw(2) << timeinfo->tm_mon + 1 << "/"
        << std::setfill('0') << std::setw(2) << timeinfo->tm_mday << " "
        << std::setfill('0') << std::setw(2) << timeinfo->tm_hour << ":"
        << std::setfill('0') << std::setw(2) << timeinfo->tm_min << ":"
        << std::setfill('0') << std::setw(2) << timeinfo->tm_sec
        << "] ";
    std::cout << msg << std::endl;
    */
}
