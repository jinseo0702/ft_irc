#include <map>
#include <string>
#include <iostream>
#include <cassert>

//간단하게 class를 구현
using namespace std;
class Test
{
public:
    Test(){
        cout << "Hello World\n";
    };
    ~Test(){
        cout << "Good bye World\n";
    };
};

/*
map 에 할당을 하고 value를 직접해제를 했을때 값의 변화를 보기 위한 방법입니다.
compile 방법 clang++ -O0 test_dangleingptr.cpp -o tet -g
assert를 확인할 수 있다.
*/

int main(void){
    Test *one = new Test();
    map<int, Test *> Map;

    Map.insert(make_pair(1, one));
    delete one;
    one = NULL;
    cout << one << "\n";
    assert (Map[1] == NULL);
    cout << Map[1] << "\n";
    return (0);
}

/*
결과
Hello World
Good bye World
0
tet: test_dangleingptr.cpp:32: int main(): Assertion `Map[1] == NULL' failed.
[1]    3158610 IOT instruction (core dumped)  ./tet
Map[1] == NULL 이 아니기 때문에 Assertion 이 작동했다. 즉 Map[1] 은 다른 ptr값을 가지고 있다.
*/

// int main(void){
//     Test *one = new Test();
//     map<int, Test *> Map;

//     cout << "one Address is :" << one << "\n";
//     Map.insert(make_pair(1, one));
//     delete one;
//     one = NULL;
//     cout << "one Address is :" << one << "\n";
//     // assert (Map[1] == NULL);
//     cout << "Map[1] Address is :" << Map[1] << "\n";
//     return (0);
// }

/*
결과
Hello World
one Address is :0x74afeb0
Good bye World
one Address is :0
Map[1] Address is :0x74afeb0

지금 one 은 NULL이 들어갔지만 Map[1] 에는는 이전 주소가 들어가 있는걸 볼 수 있다.
즉 지금 문제가 발생을 했는데, Map[1] 은 one을 바라보는게 아니라 one의 값을 복사 한걸 확인할 수 있다.

내가가 원하는 구현을 생각을 해봤을때 one이 해제가 되고 NULL이 들어가면 Map[1]에는 NULL이 들어가야한다.
이경우는 어떻게 해결을 해야할까???? 
*/

/*
방법을 바꿔서 Map[1]에서 해제를하고 one을 바라본다면?
*/


// int main(void){
//     Test *one = new Test();
//     map<int, Test *> Map;

//     cout << "one Address is :" << one << "\n";
//     Map.insert(make_pair(1, one));
//     delete Map[1];
//     Map[1] = NULL;
//     cout << "one Address is :" << one << "\n";
//     cout << "Map[1] Address is :" << Map[1] << "\n";
//     return (0);
// }

/*
결과!
Hello World
one Address is :0xd406eb0
Good bye World
one Address is :0xd406eb0
Map[1] Address is :0
*/
