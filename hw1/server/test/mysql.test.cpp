#include "class_mysql.h"
#include <iostream>
using namespace std;
int main(){
	CLASS_MYSQL mysql("140.113.240.138", "nctuoj", "yavaf2droyPo", "nctuoj");
    auto re = mysql.query("SELECT * FROM `user`");
    cout << re.size() << endl;

    for(auto x : re){
        cout << x["id"] << endl;
    }
}
