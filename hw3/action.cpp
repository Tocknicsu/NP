#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/uio.h>
using namespace std;
#define F_CONNECTING 0
#define F_READING 1
#define F_WRITING 2
#define F_DONE 3
map<string, string> env;
void start_html();
void parse_env();
void header();
void body();
void end_html();
void client();
void myprint(int, string, bool bolder = false);
char sent_buffer[6][65536];
int buffer_sented[6];

int main(int argc, char **argv){
	cout << "Content-type: text/html" << endl << endl;
	start_html();
	parse_env();
	header();
	body();
	cout << "<body id='body' bgcolor=#336699>" << endl;
	cout << "</body>" << endl;
	client();
	end_html();
	return 0;
}
void start_html(){
	cout << "<html>" << endl;
}
void parse_env(){
	std::string str = getenv("QUERY_STRING");
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
}
void header(){
	cout << "<head>" << endl;
	cout << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />" << endl;
	cout << "<title>Network Programming Homework 3</title>" << endl;
	cout << "</head>" << endl;
}
void body(){
	cout << "<font face=\"Courier New\" size=2 color=#FFFF99>" << endl;
	cout << "<table width=\"800\" border=\"1\">" << endl;
	cout << "<tr>" << endl;
	for(int i = 1 ; i <= 5 ; i++){
		string host = string("h")+to_string(i);
		if(env[host].size()){
			cout << "<td>" << env[string(host)] << "</td>" << endl;
		}
	}
	cout << "</tr>" << endl;
	cout << "<tr>" << endl;
	for(int i = 1 ; i <= 5 ; i++){
		string host = string("h")+to_string(i);
		if(env[host].size()){
			cout << "<td valign=\"top\" id=\"m" << i << "\"></td>" << endl;
		}
	}
	cout << "</tr>" << endl;
	cout << "</table>" << endl;
}
void end_html(){
	cout << "</font>" << endl;
	cout << "</html>" << endl;
}
void client(){
    int client_fd[6];
    int client_status[6];
    bool in_use[6] = {};
    sockaddr_in client_sin[6];
    int conn = 0;
    fd_set rfds, rs, wfds, ws;
    FD_ZERO(&rfds);
    FD_ZERO(&rs);
    FD_ZERO(&wfds);
    FD_ZERO(&ws);
    FILE *file[6];
    for(int i = 1 ; i < 6 ; i++){
		string host = string("h")+to_string(i);
		if(env[host].size()){
			if(env[host].size() >= 12 && env[host].substr(0, 12) == string("140.113.167.234234")){
				myprint(i, "Denied.");
			} else {
				in_use[i] = true;
				string port = "p" + to_string(i);
				string file_name = "f" + to_string(i);
				client_fd[i] = socket(AF_INET, SOCK_STREAM, 0);
				memset(&client_sin[i], 0, sizeof(client_sin[i]));
				client_sin[i].sin_family = AF_INET;
				client_sin[i].sin_addr = *((in_addr*)gethostbyname(env[host].c_str())->h_addr);
				client_sin[i].sin_port = htons(atoi(env[port].c_str()));
				int flags = fcntl(client_fd[i], F_GETFL, 0);
				fcntl(client_fd[i], F_SETFL, flags | O_NONBLOCK);
				connect(client_fd[i], (sockaddr*)&client_sin[i], sizeof(client_sin[i]));
				client_status[i] = F_CONNECTING;
				conn++;
				FD_SET(client_fd[i], &rs);
				file[i] = fopen(env[file_name].c_str(), "r");
			}
        } else {
            client_status[i] = F_DONE;
        }
    }
    int nfds = FD_SETSIZE;
    while (conn > 0) {
        rfds = rs;
        wfds = ws;
        select(nfds, &rfds, &wfds, (fd_set*)0, (struct timeval*)0);
        for(int i = 1 ; i < 6 ; i++){
            if (client_status[i] == F_CONNECTING && (FD_ISSET(client_fd[i], &rfds))){
                int error;
                socklen_t len = sizeof(int);
                if(getsockopt(client_fd[i], SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0 || error != 0){
                    client_status[i] = F_DONE;
                    close(client_fd[i]);
                    FD_CLR(client_fd[i], &rs);
                    FD_CLR(client_fd[i], &ws);
                    conn--;
                    cerr << "Cannot connect server." << endl;
                    myprint(i, "Cannot connect server");
                } else {
                    cerr << "connect server." << endl;
                    client_status[i] = F_READING;
                }
            } else if (client_status[i] == F_WRITING && FD_ISSET(client_fd[i], &wfds)) {
                if(buffer_sented[i] == (int)strlen(sent_buffer[i])){
                    fgets(sent_buffer[i], 65536, file[i]);
                    myprint(i, sent_buffer[i], true);
                    buffer_sented[i] = 0;
                }
                int n = write(client_fd[i], sent_buffer[i]+buffer_sented[i], (int)strlen(sent_buffer[i])-buffer_sented[i]);
                cerr << n << endl;
                buffer_sented[i] += n;
                if(buffer_sented[i] == (int)strlen(sent_buffer[i])){
                    client_status[i] = F_READING;
                    FD_SET(client_fd[i], &rs);
                    FD_CLR(client_fd[i], &ws);
                }
            } else if (client_status[i] == F_READING && FD_ISSET(client_fd[i], &rfds)) {
                char buffer[65536] = {};
                int n = read(client_fd[i], buffer, 65536);
                if(n == 0){
                    FD_CLR(client_fd[i], &rs);
                    FD_CLR(client_fd[i], &ws);
                    client_status[i] = F_DONE;
                    conn--;
                    cerr << "end" << endl;
                    continue;
                } else {
                    myprint(i, buffer);
                    for(int j = 0 ; j < n ; j++){
                        if(buffer[j] == '%'){
                            client_status[i] = F_WRITING;
                            FD_SET(client_fd[i], &ws);
                            FD_CLR(client_fd[i], &rs);
                        }
                    }
                }
            }
        }
    }
}
void myprint(int id, string str, bool bolder){
	cout << "<script>document.all['m" << id << "'].innerHTML += \"";
	if(bolder) cout << "<b>";
	for(int i = 0 ; i < (int)str.size() ; i++){
		if(str[i] == '\n') cout << "<br>";
		else if(str[i] == '\r') continue;
		else if(str[i] == '<') cout << "&lt;";
		else if(str[i] == '>') cout << "&gt;";
		else if(str[i] == '&') cout << "&amp;";
		else if(str[i] == '"') cout << "&quot;";
        else if(str[i] == ' ') cout << "&nbsp;";
		else cout << str[i];
	}
	if(bolder) cout << "</b>";
	cout << "\"</script>" << flush;
}
