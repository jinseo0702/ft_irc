#ifndef TOTALDATABASE_HPP
#define TOTALDATABASE_HPP

#include "./SharedPtr.hpp"
#include <map>

template <typename SpType>
class TotalDatabase {
    private:
        int userId;
        std::map<int, SharedPtr<SpType>> UserData;
    public:
        typedef typename std::map<int, SharedPtr<SpType>>::iterator it;
        typedef typename std::map<int, SharedPtr<SpType>>::const_iterator const_it;
    public:
        TotalDatabase();
        TotalDatabase(const TotalDatabase &obj);
        TotalDatabase &operator=(const TotalDatabase &obj);
        ~TotalDatabase();

        //addUser Data
        void addUser(SpType *newdata);
        void addUserWithId(SpType *newdata);
        // Getter
        it getUserData(const int id);
        const_it getUserData(const int id) const;
        // Check id is real
        bool countData(const int id) const;
        // MakeIterator
        it begin();
        const_it begin() const;
        it end();
        const_it end() const;
        //count number of data
        int sizeData() const;
        // erase data
        void eraseData(const int id);
};

template <typename SpType>
TotalDatabase<SpType>::TotalDatabase() : userId(0){
    this->UserData.clear();
};

template <typename SpType>
TotalDatabase<SpType>::TotalDatabase(const TotalDatabase &obj) : userId(obj.userId), UserData(obj.UserData){
};

template <typename SpType>
TotalDatabase<SpType> &TotalDatabase<SpType>::operator=(const TotalDatabase &obj){
    if (this != &obj){
        this->userId = obj.userId;
        this->UserData = obj.UserData;
    }
    return (*this);
};

template <typename SpType>
TotalDatabase<SpType>::~TotalDatabase(){
    this->UserData.clear();
}

template <typename SpType>
void
TotalDatabase<SpType>::addUser(SpType *newdata){
    SharedPtr<SpType> temp(newdata);
    this->UserData.insert(std::make_pair(this->userId, temp));
    ++(this->userId);
}

//이 함수를 사용하기 위해서는 반드시 Value로 들어가는 자료형은 setId라는 메소드를 가져야합니다.
//메소드를 가지지 않으면 동작 하지 않습니다.
template <typename SpType>
void
TotalDatabase<SpType>::addUserWithId(SpType *newdata){
    SharedPtr<SpType> temp(newdata);
    this->UserData.insert(std::make_pair(this->userId, temp));
    newdata->setId(this->userId);
    ++(this->userId);
}

template <typename SpType>
typename TotalDatabase<SpType>::it
TotalDatabase<SpType>::getUserData(const int id){
    return (this->UserData.find(id));
};

template <typename SpType>
typename TotalDatabase<SpType>::const_it 
TotalDatabase<SpType>::getUserData(const int id) const{
    return (this->UserData.find(id));
};

template <typename SpType>
bool 
TotalDatabase<SpType>::countData(const int id) const{
    return (this->UserData.count(id));
};

template <typename SpType>
typename TotalDatabase<SpType>::it 
TotalDatabase<SpType>::begin(){
    return (this->UserData.begin());
};

template <typename SpType>
typename TotalDatabase<SpType>::const_it  
TotalDatabase<SpType>::begin() const{
    return (this->UserData.begin());
};

template <typename SpType>
typename TotalDatabase<SpType>::it 
TotalDatabase<SpType>::end(){
    return (this->UserData.end());
};

template <typename SpType>
typename TotalDatabase<SpType>::const_it  
TotalDatabase<SpType>::end() const{
    return (this->UserData.end());
};

template <typename SpType>
int
TotalDatabase<SpType>::sizeData() const{
    return (this->UserData.size());
};

template <typename SpType>
void
TotalDatabase<SpType>::eraseData(const int id){
    TotalDatabase<SpType>::it it = this->UserData.find(id);
    if (it != this->UserData.end()){
        this->UserData.erase(this->UserData.find(id));
    }
};

#endif