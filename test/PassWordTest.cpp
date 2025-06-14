#include "../include/Password.hpp"

#include <string>
#include <iostream>

using namespace std;

int main(){

    bool flag = true;
    Password pass;
    pass.setisPasswordSet(true);
    while (flag)
    {
        string str1;
        string str2;
        cout << "insert PassWord : ";
        cin >> str1;
        std::string temp = str1;
        pass.setPwd(str1);    
        cout << "str1 is :" << temp <<"\n";
        cout << "hash is :" << pass.getHash() <<"\n";

        cout << "insert string2 : ";
        cin >> str2;
        cout << "str2 is :" << str2 <<"\n";

        cout << "Check Password st1 ans str2 is equal? : "<<std::boolalpha << pass.CheckPassword(str2) <<"\n";
        cout << "insert 1(true) or 0(flase) : ";
        cin >> flag;
    }
    
    cout << "Bye";
    return (0);
}