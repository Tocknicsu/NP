#include "sh.h"

SH::SH(){
    nick_name = std::string("(no name)");
    in_use = false;
    PATH = "bin:.";
}
void SH::change_dir(){
    chdir("./ras");
    setenv("PATH", PATH.c_str(), 1);
}
void SH::recover_dir(){
    chdir("../");
}
void SH::init(){
    nick_name = std::string("(no name)");
    PATH = "bin:.";
    //setenv("PATH", "bin:.", 1);
    m_count = 0;
    for(auto x : pipemap){
        close(x.second.pip[0]);
        close(x.second.pip[1]);
    }
    pipemap.clear();
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
void SH::change_fd(int fd){
    dup2(fd, STDOUT_FILENO);
}
void SH::BoardCast(std::string str){
    for(int i = 1 ; i <= 30 ; i++){
        SH &it = clients[i];
        if(it.in_use){
            change_fd(it.socket_fd);
            std::cout << str << std::endl;
        }
    }
    change_fd(socket_fd);
}
void SH::who(){
    std::cout << "<ID>\t<nickname>\t<IP/port>\t<indicate me>" << std::endl;
    for(int i = 1 ; i <= 30 ; i++){
        SH &it = clients[i];
        if(it.in_use)
            std::cout << it.id << "\t" << it.nick_name << "\t" << it.ip << "/" << it.port << (it.id == id ? "\t<-me" : "") << std::endl;
    }
}

void SH::name(std::string _name){
    for(int i = 1 ; i <= 30 ; i++){
        SH &it = clients[i];
        if(it.in_use && it.nick_name == _name){
            std::cout << "*** User '" << _name << "' already exists. ***" << std::endl;
            return;
        }
    }
    nick_name = _name;
    BoardCast(std::string("*** User from ")+ip+"/"+std::to_string(port)+" is named '"+nick_name+"'. ***");
}
void SH::Exit(){
    BoardCast(std::string("*** User '")+nick_name+"' left. ***");
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
        if(parse_cmd[1] == "PATH")
            PATH = parse_cmd[2].c_str();
        setenv(parse_cmd[1].c_str(),parse_cmd[2].c_str(),1);
    } else if(parse_cmd[0] == std::string("printenv")){
        std::cout << parse_cmd[1].c_str() << "=" << getenv(parse_cmd[1].c_str()) << std::endl;
    } else if(parse_cmd[0] == std::string("who")){
        who();
    } else if(parse_cmd[0] == std::string("tell")){
        int to_id = atoi(parse_cmd[1].c_str());
        if(clients[to_id].in_use == true){
            change_fd(clients[to_id].socket_fd);
            std::string message;
            space = 0;
            for(int i = 0 ; i < (int)arg.size() ; i++){
                if(space == 0 && arg[i] == ' ') space++;
                if(space == 1 && arg[i] != ' ') space++;
                if(space == 2) message.push_back(arg[i]);
            }
            std::cout << "*** " << nick_name << " told you ***: " << message << std::endl;
            change_fd(socket_fd);
        } else {
            std::cout << "*** Error: user #" << to_id << " does not exist yet. ***" << std::endl;
        }
    } else if(parse_cmd[0] == std::string("yell")){
        BoardCast(std::string("*** ")+nick_name+" yelled ***: " + arg);
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
                if((*global_pipe)[global_out].in_use == true){
                    std::cout << "*** Error: public pipe #" << global_out << " already exists. ***" << std::endl;
                    ok = false;
                    break;
                } else {
                    BoardCast(std::string("*** ") + nick_name + " (#"+std::to_string(id)+") just piped '"+source_cmd+"' ***");
                    pipe((*global_pipe)[global_out].pip);
                    (*global_pipe)[global_out].in_use = true;
                }
            }
            if(parse_cmd[i][k][0] == '<' && parse_cmd[i][k].size() > 1){
                global_in = atoi(parse_cmd[i][k].substr(1, parse_cmd[i][k].size()-1).c_str());
                if((*global_pipe)[global_in].in_use == false){
                    std::cout << "*** Error: public pipe #" << global_in << " does not exist yet. ***" << std::endl;
                    ok = false;
                    break;
                } else {
                    BoardCast(std::string("*** ") + nick_name + " (#"+std::to_string(id)+") just received via '"+source_cmd+"' ***");
                    (*global_pipe)[global_in].in_use = false;
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
            if(global_in != -1){
                close((*global_pipe)[global_in].pip[0]);
                close((*global_pipe)[global_in].pip[1]);
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
                    dup2((*global_pipe)[global_out].pip[1], 1);
//                    dup2((*global_pipe)[global_out].pip[1], 2);
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
                }
                if(global_in != -1){
                    dup2((*global_pipe)[global_in].pip[0], 0);
                    close((*global_pipe)[global_in].pip[0]);
                    close((*global_pipe)[global_in].pip[1]);
                }
            } else {
                dup2(pip[i-1][0], 0);
                close(pip[i-1][0]), close(pip[i-1][1]);
            }
            for(int i = 0 ; i < 128 ; i++){
                if((*global_pipe)[i].in_use){
                    close((*global_pipe)[i].pip[0]);
                    close((*global_pipe)[i].pip[1]);
                }
            }

            for(auto x : pipemap){
                close(x.second.pip[0]);
                close(x.second.pip[1]);
            }
            external(parse_cmd[i]);
        }
        int status;
        waitpid(pid, &status, 0);
    }
    if(num) m_count++;
    return 0;
}
void SH::welcome(){
    std::cout << "****************************************" << std::endl;
    std::cout << "** Welcome to the information server. **" << std::endl;
    std::cout << "****************************************" << std::endl;
    BoardCast("*** User '"+nick_name+"' entered from "+ip+"/"+std::to_string(port)+". ***");
}
void SH::prompt(){
    std::cout << "% " << std::flush;
}
