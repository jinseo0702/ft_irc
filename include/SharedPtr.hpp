#ifndef SHAREDPTR_HPP
#define SHAREDPTR_HPP

#include <iostream>
#include <cassert>

template <typename element_type>
class SharedPtr
{
private:
    element_type *ptr;
    int *refCount;
private:
    void    cleanup(){
        // refCount 가 NULL 이 아니라면 Pointer가제대로 동작
        // 그리고 *this->refCount가 0이라면 모두 해제가 되야 하는 상황이다.
        // 근데 1 밑으로 는 떨어트리지 말자 왜냐하면 SharedPtr은 살아 있기때문에 메모리 접근이 가능해서 오류가 날 수 있다.
        // 순환 참조 조심
        if (this->refCount && --(*this->refCount) == 0){
            delete this->ptr;
            delete this->refCount;
        }
    };
public:
    // p가 new 로 할당되는 경우만 생각합니다.
    explicit SharedPtr(element_type *p = NULL): ptr(p){
        if (p != NULL){
            this->refCount = new int(1);
        }
        else{
            //만약 p가 할당 되지 않은 상황으로 들어온다면
            this->refCount = NULL;
        }
    };
    SharedPtr(const SharedPtr &obj): ptr(obj.ptr), refCount(obj.refCount){
        if (this->refCount != NULL){
            ++(*this->refCount);
        }        
    };
    SharedPtr& operator=(const SharedPtr &obj){
        if (this != &obj){
            cleanup();//대입 연산을 한다는건 내가 가지고 있는 Pointer의 권한 을 해제 한다는 의미이다. 내가 가진 Pointer를 깨끗하게 만들어주도록 하자.
            this->ptr = obj.ptr;
            this->refCount = obj.refCount;
            if (this->refCount != NULL){
                ++(*this->refCount);
            }
        }
        return (*this);
    }
    ~SharedPtr(){
        cleanup();
    };
    element_type& operator*() const{
        //asseert 는 parameter가 false일때 프로그램을 종료 시킨다.
        //assert는 디버깅을 위한 코드로, release 모드에서는 동작하지 않는다.
        assert(this->ptr != NULL);
        return (*this->ptr);
    };
    element_type* operator->() const{
        assert(this->ptr != NULL);
        return (this->ptr);
    };
    void    reset(){
        cleanup();
        this->ptr = NULL;
        this->refCount = NULL;
    };
    void   set(element_type *p){
        cleanup();
        this->ptr = p;
        if (p != NULL){
            this->refCount = new int(1);
        }
        else{
            this->refCount = NULL;
        }
    };
    element_type *get() const{
        return (this->ptr);
    };
    // 참조 카운트가 0이 아닐때는 true
    // 참조 카운트가 0일때는 false
    int use_count() const{
        if (this->refCount != NULL){
            return (*this->refCount);
        }
        return (0);
    };
    bool is_valid() const{
        return (this->ptr != NULL);
    }
};

#endif