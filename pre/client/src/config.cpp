#include "config.h"

CONFIG::CONFIG(){
	std::string config_file_path = root_dir() + std::string("/config");
    std::ifstream ifs;
	ifs.open( config_file_path.c_str() );
    ifs >> m_ip;
	ifs >> m_port;
	ifs.close();
}
CONFIG::~CONFIG(){
}
std::string CONFIG::ip(){
    return m_ip;
}
int CONFIG::port(){
	return m_port;
}
