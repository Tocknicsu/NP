#include "config.h"

CONFIG::CONFIG(){
	std::string config_file_path = root_dir() + std::string("/config");
    std::ifstream ifs;
	ifs.open( config_file_path.c_str() );
	ifs >> m_port;
	ifs >> m_max_wait_listen;
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
