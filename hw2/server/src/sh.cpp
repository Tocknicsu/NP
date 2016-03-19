#include "sh.h"

SH::SH(){
}
void SH::change_dir(){
    chdir("./ras");
}
void SH::recover_dir(){
    chdir("../");
}
void SH::init(int _key){
    change_dir();
    setenv("PATH", "bin:.", 1);
    m_count = 0;
    for(auto x : pipemap){
        close(x.second.pip[0]);
        close(x.second.pip[1]);
    }
    pipemap.clear();
    KEY = _key;
}
bool can_exec(std::string file){
    std::string path = getenv("PATH");
    std::vector<std::string> dir;
    dir.push_back("");
    for(int i = 0 ; i < (int)path.size() ; i++){
        if(path[i]==':') dir.push_back("");
        else dir.back().push_back(path[i]);
    }
    for(int i = 0 ; i < (int)dir.size() ; i++){
        std::string look_file = dir[i] + std::string("/") + file;
        struct stat st;
        if (stat(look_file.c_str(), &st) >= 0)
            if ((st.st_mode & S_IEXEC) != 0)
                return true;
    }
    return false;
}
std::string SH::get_cwd(){
    char buffer[1024];
    getcwd(buffer, 1024);
    return std::string(buffer);
}
std::vector<std::string> SH::parse_single_cmd(std::string str){
    std::vector<std::string> cmd;
    cmd.push_back("");
    for(int i = 0 ; i < (int)str.size() ; i++){
        if(str[i] == 10 || str[i] == 13) continue;
        if(str[i] == ' '){
            if(cmd.back().size())
                cmd.push_back("");
        } else {
            cmd.back().push_back(str[i]);
        }
    }
    while(cmd.size() && !cmd.back().size())
        cmd.pop_back();
    return cmd;
}
void SH::external(std::vector<std::string> cmd){
    char **args;
    args = new char*[cmd.size()+1];
    for(int i = 0 ; i < (int)cmd.size() ; i++){
        args[i] = new char[cmd[i].size()+1];
        strcpy(args[i], cmd[i].c_str());
    }
    args[cmd.size()] = 0;
    execvp(cmd[0].c_str(), args);
    exit(1);
}
void SH::clear_buffer(){
    sem_wait(KEY);
    for(int i = 0 ; i < msgptr->clients[id].msg.num ; i++){
        std::cout << msgptr->clients[id].msg.text[i] << std::endl << std::flush;
    }
    msgptr->clients[id].msg.num = 0;
    sem_signal(KEY);
}
void SH::Sent_Message(int _id, std::string str){
    sem_wait(KEY);
    strcpy(msgptr->clients[_id].msg.text[msgptr->clients[_id].msg.num], str.c_str());
    msgptr->clients[_id].msg.num++;
    sem_signal(KEY);
}
void SH::BoardCast(std::string str){
    for(int i = 1 ; i <= 30 ; i++){
        if( i == id ){
            std::cout << str << std::endl;
        } else if(msgptr->clients[i].in_use){
            Sent_Message(i, str);
        }
    }
}
void SH::who(){
    sem_wait(KEY);
    std::cout << "<ID>\t<nickname>\t<IP/port>\t<indicate me>" << std::endl;
    for(int i = 1 ; i <= 30 ; i++){
        if(msgptr->clients[i].in_use){
            std::cout << i << "\t" << msgptr->clients[i].nick_name << "\t" << msgptr->clients[i].ip << "/" << msgptr->clients[i].port << (i==id?"\t<-me":"")<< std::endl;
        }
    }
    sem_signal(KEY);
}

void SH::name(std::string _name){
    for(int i = 1 ; i <= 30 ; i++){
        if(msgptr->clients[i].in_use && strcmp(msgptr->clients[i].nick_name, _name.c_str()) == 0){
            std::cout << "*** User '" << _name << "' already exists. ***" << std::endl;
            return;
        }
    }
    strcpy(msgptr->clients[id].nick_name, _name.c_str());
    BoardCast(std::string("*** User from ")+msgptr->clients[id].ip+"/"+std::to_string(msgptr->clients[id].port)+" is named '"+msgptr->clients[id].nick_name+"'. ***");
}
void SH::Exit(){
    BoardCast(std::string("*** User '")+msgptr->clients[id].nick_name+"' left. ***");
}
int SH::internal(std::string cmd){
    std::vector<std::string> parse_cmd = parse_single_cmd(cmd);
    std::string arg;
    int space = 0;
    for(int i = 0 ; i < (int)cmd.size() ; i++){
        if(cmd[i] == 10 || cmd[i] == 13) continue;
        if(space == 0 && cmd[i] == ' ') space++;
        if(space == 1 && cmd[i] != ' ') space++;
        if(space == 2)
            arg.push_back(cmd[i]);
    }
    if(parse_cmd.size()==0) return false;
    if(parse_cmd[0] == std::string("exit")){
        Exit();
        return -1;
    } else if(parse_cmd[0] == std::string("setenv")){
        setenv(parse_cmd[1].c_str(),parse_cmd[2].c_str(),1);
    } else if(parse_cmd[0] == std::string("printenv")){
        std::cout << parse_cmd[1].c_str() << "=" << getenv(parse_cmd[1].c_str()) << std::endl;
    } else if(parse_cmd[0] == std::string("who")){
        who();
    } else if(parse_cmd[0] == std::string("tell")){
        int to_id = atoi(parse_cmd[1].c_str());
        if(msgptr->clients[to_id].in_use == true){
            std::string message;
            space = 0;
            for(int i = 0 ; i < (int)arg.size() ; i++){
                if(space == 0 && arg[i] == ' ') space++;
                if(space == 1 && arg[i] != ' ') space++;
                if(space == 2) message.push_back(arg[i]);
            }
            Sent_Message(to_id, std::string("*** ") + msgptr->clients[id].nick_name + " told you ***: " + message);
        } else {
            std::cout << "*** Error: user #" << to_id << " does not exist yet. ***" << std::endl;
        }
    } else if(parse_cmd[0] == std::string("yell")){
        BoardCast(std::string("*** ")+msgptr->clients[id].nick_name+" yelled ***: " + arg);
    } else if(parse_cmd[0] == std::string("name")){
        name(arg);
    } else {
        return false;
    }
    return true;
}
void SH::create_map_pipe(int num){
    if(pipemap.find(num) == pipemap.end()){
        pipemap[num] = PIPE();
        pipe(pipemap[num].pip);
    }
}
bool file_exists(std::string str){
    FILE* fp = fopen(str.c_str(), "r");
    if (fp) {
        fclose(fp);
        return true;
    } else {
        return false;
    }
}
int SH::exec(std::string cmd){
    std::string source_cmd = cmd;
    while(source_cmd.size() && (source_cmd.back() == 13 || source_cmd.back() == 10)) source_cmd.pop_back();
    int result = internal(cmd);
    if(result) return result;
    std::stringstream ss(cmd);
    std::vector<std::string> cmds;
    cmds.push_back("");
    while(ss >> cmd){
        if(cmd != "|")
            cmds.back() += (cmds.back().size()?" ":"")+cmd;
        else
            cmds.push_back("");
    }
    std::vector<std::vector<std::string> > parse_cmd;

    int pip_num = cmds.size() - 1;
    int pip[pip_num][2];

    int last_stderr = -1;
    int last_stdout = -1;

    int num = 0;

    for(int i = 0 ; i < (int)cmds.size() ; i++){
        parse_cmd.push_back(parse_single_cmd(cmds[i]));
        int global_in = -1;
        int global_out = -1;
        if(i == cmds.size() - 1){   //last command pipe
            for(int k = 0 ; k < 2 ; k++){
                if(parse_cmd[i].size() > 1 && parse_cmd[i].back()[0] == '|'){
                    last_stdout = atoi(parse_cmd[i].back().substr(1, parse_cmd[i].back().size()-1).c_str());
                    parse_cmd[i].pop_back();
                }
                if(parse_cmd[i].size() > 1 && parse_cmd[i].back()[0] == '!'){
                    last_stderr = atoi(parse_cmd[i].back().substr(1, parse_cmd[i].back().size()-1).c_str());
                    parse_cmd[i].pop_back();
                }
            } 
            if(last_stderr != -1){
                create_map_pipe(m_count+last_stderr);
            }
            if(last_stdout != -1){
                create_map_pipe(m_count+last_stdout);
            }
        } else {
            pipe(pip[i]);
        }
        bool ok = true;
        for(int k = 0 ; k < (int)parse_cmd[i].size() ; k++){
            if(parse_cmd[i][k][0] == '>' && parse_cmd[i][k].size() > 1){
                global_out = atoi(parse_cmd[i][k].substr(1, parse_cmd[i][k].size()-1).c_str());
                std::string file_name = std::string("/tmp/tocknicsu_fifo.")+std::to_string(global_out);
                if(file_exists(file_name)){
                    std::cout << "*** Error: public pipe #" << global_out << " already exists. ***" << std::endl;
                    ok = false;
                    break;
                } else {
                    BoardCast(std::string("*** ") + msgptr->clients[id].nick_name + " (#"+std::to_string(id)+") just piped '"+source_cmd+"' ***");
                }
            }
            if(parse_cmd[i][k][0] == '<' && parse_cmd[i][k].size() > 1){
                global_in = atoi(parse_cmd[i][k].substr(1, parse_cmd[i][k].size()-1).c_str());
                std::string file_name = std::string("/tmp/tocknicsu_fifo.")+std::to_string(global_in);
                if(file_exists(file_name)){
                    BoardCast(std::string("*** ") + msgptr->clients[id].nick_name + " (#"+std::to_string(id)+") just received via '"+source_cmd+"' ***");
                    msgptr->global_pipe[global_in] = false;
                } else {
                    std::cout << "*** Error: public pipe #" << global_in << " does not exist yet. ***" << std::endl;
                    ok = false;
                    break;
                }
            }
        }
        for(int k = 0 ; k < (int)parse_cmd[i].size() ; k++){
            if(parse_cmd[i][k][0] == '<' && parse_cmd[i][k].size() > 1){
                parse_cmd[i].erase(parse_cmd[i].begin()+k);
            }
        }
        for(int k = 0 ; k < (int)parse_cmd[i].size() ; k++){
            if(parse_cmd[i][k][0] == '>' && parse_cmd[i][k].size() > 1){
                parse_cmd[i].erase(parse_cmd[i].begin()+k);
            }
        }
        if(!ok) break;
        if(!can_exec(parse_cmd[i][0])){
            std::cout << "Unknown command: [" << parse_cmd[i][0] << "]." << std::endl;
            break;
        }
        num++;
        int pid = fork();
        if(pid){
            if(i == 0){
                if(pipemap.find(m_count) != pipemap.end()){
                    close(pipemap[m_count].pip[1]);
                    close(pipemap[m_count].pip[0]);
                }
            } else {
                close(pip[i-1][0]), close(pip[i-1][1]);
            }
        } else {
            if(i == cmds.size() - 1){
                if(parse_cmd[i].size() > 2 && parse_cmd[i][parse_cmd[i].size()-2] == ">"){
                    freopen(parse_cmd[i].back().c_str(), "w", stdout);
                    parse_cmd[i].pop_back();
                    parse_cmd[i].pop_back();
                }
                if(last_stdout != -1){
                    dup2(pipemap[m_count+last_stdout].pip[1], 1);
                }
                if(last_stderr != -1){
                    dup2(pipemap[m_count+last_stderr].pip[1], 2);
                }
                if(global_out != -1){
                    int fd = open( (std::string("/tmp/tocknicsu_fifo.")+std::to_string(global_out)).c_str(), O_WRONLY | O_CREAT, 0666);
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                }
            } else {
                dup2(pip[i][1], 1);
                close(pip[i][0]), close(pip[i][1]);
            }
            /* in */
            if(i == 0){
                if(pipemap.find(m_count) != pipemap.end()){
                    dup2(pipemap[m_count].pip[0], 0);
                    close(pipemap[m_count].pip[1]);
                    close(pipemap[m_count].pip[0]);
                } else if(global_in != -1){
                    freopen((std::string("/tmp/tocknicsu_fifo.")+std::to_string(global_in)).c_str(), "r", stdin);
                } else {
                    close(STDIN_FILENO);
                }
            } else {
                dup2(pip[i-1][0], 0);
                close(pip[i-1][0]), close(pip[i-1][1]);
            }

            for(auto x : pipemap){
                close(x.second.pip[0]);
                close(x.second.pip[1]);
            }
            external(parse_cmd[i]);
        }
        int status;
        waitpid(pid, &status, 0);
        if(global_in != -1){
            remove((std::string("/tmp/tocknicsu_fifo.")+std::to_string(global_in)).c_str());
        }
    }
    if(num) m_count++;
    return 0;
}
void SH::welcome(){
    std::cout << "****************************************" << std::endl;
    std::cout << "** Welcome to the information server. **" << std::endl;
    std::cout << "****************************************" << std::endl;
    BoardCast(std::string("*** User '")+msgptr->clients[id].nick_name+"' entered from "+msgptr->clients[id].ip+"/"+std::to_string(msgptr->clients[id].port)+". ***");
}
void SH::prompt(){
    std::cout << "% " << std::flush;
}
