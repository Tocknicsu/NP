#include <winsock2.h>
#include <windows.h>
#include <bits/stdc++.h>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
SOCKET m_server=NULL;
sockaddr sockAddrClient;
int nPort=1234;
#define WM_SOCKET		(WM_USER+100)
#define TYPE_NEW_CONNECTION 0
#define TYPE_CGI 1
#define TYPE_HTML 2
#define TYPE_CLIENT 3
LRESULT CALLBACK WinProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
struct wrapper_data{
	int type;
	SOCKET m_cgi;
	FILE* fp;
	string buffer;
	int server_num;
	int status;
	wrapper_data(){}
};
void easy_send(SOCKET& to, string& str){
	//int n = send(to, str.substr(send_len, str.size()-send_len).c_str(), min((int)str.size()-send_len, 5), 0);
	if(str.size()){
		int n = send(to, str.c_str(), (int)str.size(), 0);
		if(n >= 0)
			str = str.substr(n, str.size() - n);
	}
	
}
map<SOCKET, wrapper_data> sockets;

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nShowCmd){
	WNDCLASSEX wClass;
	ZeroMemory(&wClass,sizeof(WNDCLASSEX));
	wClass.cbClsExtra=NULL;
	wClass.cbSize=sizeof(WNDCLASSEX);
	wClass.cbWndExtra=NULL;
	wClass.hbrBackground=(HBRUSH)COLOR_WINDOW;
	wClass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wClass.hIcon=NULL;
	wClass.hIconSm=NULL;
	wClass.hInstance=hInst;
	wClass.lpfnWndProc=(WNDPROC)WinProc;
	wClass.lpszClassName="Window Class";
	wClass.lpszMenuName=NULL;
	wClass.style=CS_HREDRAW|CS_VREDRAW;
	if(!RegisterClassEx(&wClass)){
		int nResult=GetLastError();
		MessageBox(NULL,
			"Window class creation failed\r\nError code:",
			"Window Class Failed",
			MB_ICONERROR);
	}
	HWND hWnd=CreateWindowEx(NULL,
			"Window Class",
			"Winsock Async Server",
			WS_OVERLAPPEDWINDOW,
			200,
			200,
			640,
			480,
			NULL,
			NULL,
			hInst,
			NULL);
	if(!hWnd){
		int nResult=GetLastError();

		MessageBox(NULL,
			"Window creation failed\r\nError code:",
			"Window Creation Failed",
			MB_ICONERROR);
	}
    ShowWindow(hWnd,nShowCmd);
	MSG msg;
	ZeroMemory(&msg,sizeof(MSG));
	while(GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK WinProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam){
	switch(msg){
		case WM_CREATE: {
			WSADATA WsaDat;
			int nResult=WSAStartup(MAKEWORD(2,2),&WsaDat);
			if(nResult!=0){
				MessageBox(hWnd, "Winsock initialization failed", "Critical Error", MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}
			m_server=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			if(m_server==INVALID_SOCKET){
				MessageBox(hWnd, "Winsock initialization failed", "Critical Error", MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}
			SOCKADDR_IN SockAddr;
			SockAddr.sin_port=htons(nPort);
			SockAddr.sin_family=AF_INET;
			SockAddr.sin_addr.s_addr=htonl(INADDR_ANY);
			if(bind(m_server,(LPSOCKADDR)&SockAddr,sizeof(SockAddr))==SOCKET_ERROR){
				MessageBox(hWnd,"Unable to bind socket","Error",MB_OK);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}
			nResult=WSAAsyncSelect(m_server,hWnd,WM_SOCKET,(FD_ACCEPT));
			if(nResult){
				MessageBox(hWnd,"WSAAsyncSelect failed","Critical Error",MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}
			if(listen(m_server,(16))==SOCKET_ERROR){
				MessageBox(hWnd,"Unable to listen!","Error",MB_OK);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}
			break;
		}
		case WM_DESTROY:{
			PostQuitMessage(0);
			shutdown(m_server,SD_BOTH);
			closesocket(m_server);
			WSACleanup();
			return 0;
		}
		case WM_SOCKET:{
			switch(WSAGETSELECTEVENT(lParam)){
				case FD_READ:{
					wrapper_data &data = sockets[wParam]; 
					cout << "reading data type: " << flush << data.type << endl;
					if(data.type == TYPE_NEW_CONNECTION){
						char buffer[65536] = {};
						recv(wParam, buffer, sizeof(buffer), 0);
						std::stringstream ss(buffer);
						string query_type, url, file_path, query_string, file_extension;
						ss >> query_type >> url;
						if(url.size() >= 1)
					   	file_path = url.substr(1, url.find_last_of('?')-1);
						if(file_path.find(".") != string::npos)
							file_extension = file_path.substr(file_path.find_last_of('.') + 1, file_path.size() - file_path.find_last_of('.'));
						if(url.find("?") != string::npos)
							query_string = url.substr(url.find_last_of('?') + 1, url.size() - url.find_last_of('?'));
						while(query_string.find("%2F") != string::npos){
							query_string.replace(query_string.find("%2F"), 3, "/");
						}
						cout << query_type << ' ' << file_path << ' ' << query_string << ' ' << file_extension << endl;
						data.buffer = string("HTTP/1.1 200 OK\nContent-type: text/html\n\n");
						string str = query_string;
						if(file_extension == string("cgi")){
							map<string, string> env;
							vector<string> tmp_env;
							tmp_env.push_back("");
							for(int i = 0 ; i < (int)str.size() ; i++){
								if(str[i] == '&')
									tmp_env.push_back("");
								else
									tmp_env.back().push_back(str[i]);
							}
							for(int i = 0 ; i < (int)tmp_env.size() ; i++){
								string tmp[2];
								bool flag = 0;
								for(int j = 0 ; j < (int)tmp_env[i].size() ; j++){
									if(tmp_env[i][j] == '=')
										flag++;
									else
										tmp[flag].push_back(tmp_env[i][j]);
								}
								env[tmp[0]] = tmp[1];
							}
							data.type = TYPE_CGI;
							data.buffer += string("<html>\n");
							data.buffer += string("<head>\n");
							data.buffer += string("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n");
							data.buffer += string("<title>Network Programming Homework 3</title>\n");
							data.buffer += string("</head>\n");
							data.buffer += string("<body id='body' bgcolor=#336699>\n");
							data.buffer += string("<font face=\"Courier New\" size=2 color=#FFFF99>\n");
							data.buffer += string("<table width=\"800\" border=\"1\">\n");
							data.buffer += string("<tr>\n");
							for(int i = 1 ; i <= 5 ; i++){
								string host = string("h")+to_string(i);
								if(env[host].size()){
									data.buffer += string(string("<td>") + env[string(host)] + string("</td>\n"));
								}
							}
							data.buffer += string("</tr>\n");
							data.buffer += string("<tr>\n");
							for(int i = 1 ; i <= 5 ; i++){
								string host = string("h")+to_string(i);
								if(env[host].size()){
									data.buffer += string(string("<td valign=\"top\" id=\"m") + to_string(i) + string("\"></td>\n"));
								}
							}
							data.buffer += string("</tr>\n");
							data.buffer += string("</table>\n</body>\n");
							data.buffer += string("</font>\n");
							data.buffer += string("</html>\n");
							data.server_num = 0; 
							for(int i = 1 ; i <= 5 ; i++){
								string host = string("h") + to_string(i);
								if(env[host].size()){
									data.server_num++;
									string port = "p" + to_string(i);
									string file_name = "f" + to_string(i);
									SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
									sockaddr_in client_sin;
									client_sin.sin_family = AF_INET;
            						client_sin.sin_addr = *((in_addr*)gethostbyname(env[host].c_str())->h_addr);
            						client_sin.sin_port = htons(atoi(env[port].c_str()));
            						u_long val = 1;
            						ioctlsocket(client, FIONBIO, &val);
            						connect(client, (sockaddr*)&client_sin, sizeof(client_sin));
            						sockets[client] = wrapper_data();
            						sockets[client].fp = fopen(env[file_name].c_str(), "r");
            						sockets[client].server_num = i;
            						sockets[client].m_cgi = wParam;
            						sockets[client].type = TYPE_CLIENT;
            						WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_CONNECT | FD_CLOSE);
            						cout << "create new remote waiting to conenct" << endl;
								}
							}
						} else {
							data.type = TYPE_HTML;
							ifstream f(file_path.c_str());
							string str;
							while(getline(f, str)){
								data.buffer += str;
							}
							f.close();
						}
						WSAAsyncSelect(wParam, hWnd, WM_SOCKET, FD_WRITE | FD_CLOSE);
					} else if(data.type == TYPE_CLIENT){
						char buffer[65536];
						int len = recv(wParam, buffer, sizeof(buffer), 0);
						string &send_buffer = sockets[sockets[wParam].m_cgi].buffer;
						int id = sockets[wParam].server_num; 
						bool next = false;
						send_buffer += string("<script>document.all['m") + to_string(id) + string("'].innerHTML += \"");
						for(int i = 0 ; i < len ; i++){
							if(buffer[i] == '%') next = true;
						 	if(buffer[i] == '\n') send_buffer += string("<br>");
						 	else if(buffer[i] == '\r') continue;
						 	else if(buffer[i] == '<') send_buffer += string("&lt;");
						 	else if(buffer[i] == '>') send_buffer += string("&gt;");
						 	else if(buffer[i] == '&') send_buffer += string("&amp;");
						 	else if(buffer[i] == '"') send_buffer += string("&quot;");
						 	else send_buffer.push_back(buffer[i]);
						}
						send_buffer += string("\"</script>\n");
						if(next){
							string &send_remote_buffer = sockets[wParam].buffer;
							char buffer[65536];
							fgets(buffer, 65536, sockets[wParam].fp);
							cout << "next command: " << buffer << endl;
							send_remote_buffer = string(buffer); 
							WSAAsyncSelect(wParam, hWnd, WM_SOCKET, FD_WRITE | FD_CLOSE);
							
							send_buffer += string("<script>document.all['m") + to_string(id) + string("'].innerHTML += \"<b>");
							for(int i = 0 ; buffer[i] ; i++){
								if(buffer[i] == '%') next = true;
							 	if(buffer[i] == '\n') send_buffer += string("<br>");
							 	else if(buffer[i] == '\r') continue;
							 	else if(buffer[i] == '<') send_buffer += string("&lt;");
							 	else if(buffer[i] == '>') send_buffer += string("&gt;");
							 	else if(buffer[i] == '&') send_buffer += string("&amp;");
							 	else if(buffer[i] == '"') send_buffer += string("&quot;");
							 	else send_buffer.push_back(buffer[i]);
							}
							send_buffer += string("</b>\"</script>\n");
							
						}
					} else {
					}
					break;
					
				}
				case FD_WRITE:{
					wrapper_data &data = sockets[wParam]; 
					easy_send(wParam, data.buffer);
					if(data.type == TYPE_HTML && data.buffer.empty()){
						WSAAsyncSelect(wParam, hWnd, WM_SOCKET, FD_CLOSE);
						PostMessage(hWnd, WM_SOCKET, wParam, FD_CLOSE);
					} else if(data.type == TYPE_CGI && data.buffer.empty() && data.server_num == 0) {
						WSAAsyncSelect(wParam, hWnd, WM_SOCKET, FD_CLOSE);
						PostMessage(hWnd, WM_SOCKET, wParam, FD_CLOSE);
					} else if(data.type == TYPE_CLIENT && data.buffer.empty()) {
						WSAAsyncSelect(wParam, hWnd, WM_SOCKET, FD_CLOSE | FD_READ); 
					} else {
						PostMessage(hWnd, WM_SOCKET, wParam, FD_WRITE);
					}
					break;
				}
				case FD_CLOSE:{
					
					if(wParam == m_server){
						cout << "server closed" << endl; 
					} else {
						wrapper_data &data = sockets[wParam]; 
						if(data.type == TYPE_CLIENT){
							cout << "client closed" << endl;
							sockets[data.m_cgi].server_num--;
							cout << sockets[data.m_cgi].server_num << endl;
						}
						shutdown(wParam, SD_BOTH);
					}
					break;
				}
				case FD_ACCEPT:{
					SOCKADDR_IN clientAddr;
					int addrlen = sizeof(clientAddr);
					SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
					client = accept(wParam, (SOCKADDR*)&clientAddr, &addrlen);
					if (client==INVALID_SOCKET){
						cout << "error accept" << endl;
						break;
					}
					WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
					cout << "new connection" << endl;
					sockets[client] = wrapper_data();
					sockets[client].type = TYPE_NEW_CONNECTION;
					break;
				}
				case FD_CONNECT:{
					cout << "connect event happened" << endl; 
					int error, n;
					WSAAsyncSelect(wParam, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
					/*
					if(getsockopt(wParam, SOL_SOCKET, SO_ERROR, (char*)&error, &n) < 0 || error != 0){
						WSAAsyncSelect(wParam, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
						//cout << "connect error" << endl;
						//PostMessage(hWnd, WM_SOCKET, wParam, FD_CLOSE);
					} else {
						cout << "connect remote server success!" << endl;
						WSAAsyncSelect(wParam, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
					}
					*/
					break;
				}
    		}   
		}
	}
    return DefWindowProc(hWnd,msg,wParam,lParam);
}