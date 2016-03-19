#include "config.h"

CONFIG::CONFIG(){
	std::string config_file_path = root_dir() + std::string("/config");
    std::ifstream ifs;
	ifs.open( config_file_path.c_str() );
    
	if(!(ifs >> m_port)) m_port = 1739;
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
int sem_create(key_t key, int initval){
    sembuf op_lock[2] = {
        2, 0, 0,
        2, 1, SEM_UNDO
    }, op_endcreate[2] = {
        1, -1, SEM_UNDO,
        2, -1, SEM_UNDO
    };
    int id, semval;
    if(key == IPC_PRIVATE) return -1;
    else if(key == (key_t)-1) return -1;
again:
    if((id = semget(key, 3, 0666 | IPC_CREAT)) < 0) return -1;
    if(semop(id, &op_lock[0], 2) < 0){
        if(errno == EINVAL) goto again;
        std::cerr << "error" << std::endl;
    }
    if((semval = semctl(id, 1, GETVAL, 0)) < 0){
        std::cerr << "can't getval" << std::endl;
    }
    if(semval == 0){
        if(semctl(id, 0, SETVAL, initval) < 0){
            std::cerr << "gg" << std::endl;
        }
        if(semctl(id, 1, SETVAL, BIGCOUNT) < 0){
            std::cerr << "GG2" << std::endl;
        }
    }
    if(semop(id, &op_endcreate[0], 2) < 0)
        std::cerr << "create sem key end gg" << std::endl;
    return id;
}
void sem_rm(int id){
    if(semctl(id, 0, IPC_RMID, 0) < 0)
        std::cerr << "sem rm gg" << std::endl;
}
int sem_open(key_t key){
    sembuf op_open[1] = {
        1, -1, SEM_UNDO
    };
    int id;
    if(key == IPC_PRIVATE) return -1;
    else if(key == (key_t)-1) return -1;

    if( (id = semget(key, 3, 0)) < 0 ) return -1;
    if(semop(id, &op_open[0], 1) < 0){
        std::cerr << "open gg" << std::endl;
    }
    return id;
}
void sem_close(int id){
    int semval;
    sembuf op_close[3] = {
        2, 0, 0,
        2, 1, SEM_UNDO,
        1, 1, SEM_UNDO
    }, op_unlock[1] = {
        2, -1, SEM_UNDO
    };
    if(semop(id, &op_close[0], 3) < 0){
    }
    if( (semval = semctl(id, 1, GETVAL, 0)) < 0){
    }
    if(semval > BIGCOUNT){
    } else if(semval == BIGCOUNT){
        sem_rm(id);
    } else {
        if(semop(id, &op_unlock[0], 1) <0){
        }
    }
}
void sem_op(int id, int value){
    sembuf op_op[1] = {
        0, 99, SEM_UNDO
    };
    if( (op_op[0].sem_op = value) == 0){
    }
    if(semop(id, &op_op[0], 1) < 0){
    }
}
void sem_wait(int id){
    sem_op(id, -1);
}
void sem_signal(int id){
    sem_op(id, 1);
}
