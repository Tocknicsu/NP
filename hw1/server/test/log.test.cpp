#include "log.h"
using namespace std;

int main(){
    LOG log("log.log");
    log.write("test");
    return 0;
}
