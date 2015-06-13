#include "client.h"

CLIENT::CLIENT(){
}
void CLIENT::set_socket(int _client_socket){
	m_client_socket = _client_socket;
	m_addr_len = sizeof(m_addr);
	getpeername(m_client_socket, (sockaddr*)&m_addr, &m_addr_len);
	inet_ntop(AF_INET, &m_addr.sin_addr, m_ip, sizeof(m_ip));	
}
