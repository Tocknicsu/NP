#include <bits/stdc++.h>
using namespace std;


class CLASS_MYSQL_ROW : public vector< string > {
    private:
        vector<string> col;
    public:
        void push_back(const string col_name, const string col_value){
            col.push_back(col_name);
            vector<string>::push_back(col_value);
        }
        string operator[](int pos){
            return vector<string>::operator[](pos);
        }
        string operator[](string col_name){
            for(int i = 0 ; i < col.size() ; i++)
                if(col[i] == col_name)
                    return vector<string>::operator[](i);
        }
};

int main(){
    CLASS_MYSQL_ROW row;
    row.push_back("id", "3");
    cout << row[0] << endl;
    cout << row["id"] << endl;
    cout << row.size() << endl;
    return 0;
}
