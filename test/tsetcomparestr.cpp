#include <string>
#include <iostream>

using namespace std;

int main(){

    bool flag = true;
    while (flag)
    {
        string str1;
        string str2;
        cout << "insert string1 : ";
        cin >> str1;
        cout << "str1 is :" << str1 <<"\n";

        cout << "insert string2 : ";
        cin >> str2;
        cout << "str2 is :" << str2 <<"\n";

        int temp = str1 == str2;
        cout << "compare str1 and str2 : " << temp <<"\n";
        cout << "insert 1(true) or 0(flase) : ";
        cin >> flag;
    }
    
    cout << "Bye";
    return (0);
}

// JOIN fsjksfd #fdsf
// JOIN fsdfsadf