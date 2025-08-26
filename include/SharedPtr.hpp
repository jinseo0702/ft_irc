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
        if (this != &obj) {
            cleanup();
            this->ptr = obj.ptr;
            this->refCount = obj.refCount;
            if (this->refCount != NULL) {
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