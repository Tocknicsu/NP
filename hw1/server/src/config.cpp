#include "config.h"

CONFIG::CONFIG(){
	std::string config_file_path = root_dir() + std::string("/config");
    std::ifstream ifs;
	ifs.open( config_file_path.c_str() );
    
	if(!(ifs >> m_port)) m_port = 1737;
	if(!(ifs >> m_max_wait_listen)) m_max_wait_listen = 4;
	ifs.close();
}
CONFIG::~CONFIG(){
}
int CONFIG::port(){
	return m_port;
}
int CONFIG::max_wait_listen(){
	return m_max_wait_listen;
}
