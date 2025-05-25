#include "./SharedPtr.hpp"
#include <iostream>
#include <map>
#include <unistd.h>
#include "../include/user.hpp"

/*
compile 방법
clang++ -g ../srcs/user.cpp  ./main.cpp
*/

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

using namespace std;
int main(){
   { 
        SharedPtr<map<int, int>> sp(new map<int, int>());
        sp->insert(make_pair(1, 1));
        sp->insert(make_pair(2, 2));
        sp->insert(make_pair(3, 3));
        sp->insert(make_pair(4, 4));
        for (map<int, int>::iterator it = sp->begin(); it != sp->end(); ++it)
        {
            cout << "Key: " << it->first << ", Value: " << it->second << endl;
        }
    }
        cout << "-----------------------------------------------------------"<< endl;
    {
        SharedPtr<map<int, User>> sp(new map<int, User>());
        sp->insert(make_pair(1, User(1)));
        sp->insert(make_pair(2, User(2)));
        sp->insert(make_pair(3, User(3)));
        sp->insert(make_pair(4, User(4)));
        for (map<int, User>::iterator it = sp->begin(); it != sp->end(); ++it)
        {
            cout << "Key: " << it->first << ", Value: " << it->second.getFd() << endl;
        }
        cout << "use_count: " << sp.use_count() << endl;
    }
        cout << "-----------------------------------------------------------"<< endl;
    {
        SharedPtr<Test> one(new Test());
        map<int, SharedPtr<Test>> Map;

        Map.insert(make_pair(1, one));
        cout << "one Address is :" << one.get() << "\n";
        cout << "Map[1] Address is :" << Map[1].get() << "\n";
        cout << "one use_count is :" << one.use_count() << "\n";
        cout << "Map[1] use_count is :" << Map[1].use_count() << "\n";
        one.reset();
        cout << "one Address is :" << one.get() << "\n";
        cout << "Map[1] Address is :" << Map[1].get() << "\n";
        cout << "one use_count is :" << one.use_count() << "\n";
        cout << "Map[1] use_count is :" << Map[1].use_count() << "\n";
        Map[1].reset();
        cout << "one Address is :" << one.get() << "\n";
        cout << "Map[1] Address is :" << Map[1].get() << "\n";
        cout << "one use_count is :" << one.use_count() << "\n";
        cout << "Map[1] use_count is :" << Map[1].use_count() << "\n";
    }
    cout << "\n-----------------------------------------------------------\n"<< endl;

    struct C { int a; int b; };
    {
        SharedPtr<C> foo;
        SharedPtr<C> bar (new C);

        foo = bar;

        foo->a = 10;
        bar->b = 20;

        if (foo.is_valid()) std::cout << "foo: " << foo->a << ' ' << foo->b << '\n';
        if (bar.is_valid()) std::cout << "bar: " << bar->a << ' ' << bar->b << '\n';
    }


    cout << "\n-----------------------------------------------------------\n"<< endl;

    // 순환 참조 예제
    class Node {
    public:
        int data;
        SharedPtr<Node> next;
        SharedPtr<Node> parent;
        
        Node(int value) : data(value) {
            cout << "Node " << data << " created\n";
        }
        
        ~Node() {
            cout << "Node " << data << " destroyed\n";
        }
    };
    
    cout << "=== 순환 참조 테스트 시작 ===" << endl;
    {
        SharedPtr<Node> node1(new Node(1));
        SharedPtr<Node> node2(new Node(2));
        
        cout << "node1 use_count: " << node1.use_count() << endl;
        cout << "node2 use_count: " << node2.use_count() << endl;
        
        // 순환 참조 생성
        node1->next = node2;    // node1이 node2를 참조
        node2->parent = node1;  // node2가 node1을 참조
        
        cout << "순환 참조 생성 후:" << endl;
        cout << "node1 use_count: " << node1.use_count() << endl;
        cout << "node2 use_count: " << node2.use_count() << endl;
        
        cout << "스코프 종료 전..." << endl;
    }
    cout << "스코프 종료 후 - 메모리 누수 발생!" << endl;
    cout << "Node 객체들이 소멸되지 않음 (순환 참조로 인해)" << endl;
    
    cout << "\n=== 순환 참조 해결 방법 ===" << endl;
    {
        SharedPtr<Node> node3(new Node(3));
        SharedPtr<Node> node4(new Node(4));
        
        // 순환 참조 생성
        node3->next = node4;
        node4->parent = node3;
        
        cout << "순환 참조 생성 후:" << endl;
        cout << "node3 use_count: " << node3.use_count() << endl;
        cout << "node4 use_count: " << node4.use_count() << endl;
        
        // 수동으로 순환 참조 해제
        cout << "수동으로 순환 참조 해제..." << endl;
        node3->next.reset();    // 또는 node4->parent.reset();
        
        cout << "순환 참조 해제 후:" << endl;
        cout << "node3 use_count: " << node3.use_count() << endl;
        cout << "node4 use_count: " << node4.use_count() << endl;
        
        cout << "스코프 종료 전..." << endl;
    }
    cout << "스코프 종료 후 - 정상적으로 소멸됨" << endl;
    return (0);
}