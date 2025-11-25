#ifndef TOTALDATABASE_HPP
#define TOTALDATABASE_HPP

#include "./SharedPtr.hpp"
#include <map>

template <typename SpType>
class TotalDatabase {
    private:
        int userId;
        std::map<int, SharedPtr<SpType> > UserData;
    public:
        typedef typename std::map<int, SharedPtr<SpType> >::iterator it;
        typedef typename std::map<int, SharedPtr<SpType> >::const_iterator const_it;
    public:
        TotalDatabase();
        TotalDatabase(const TotalDatabase &obj);
        TotalDatabase &operator=(const TotalDatabase &obj);
        ~TotalDatabase();

        
        int addUser(SpType *newdata);
        int addUserWithId(SpType *newdata);
        int addUserWithId(SharedPtr<SpType> obj);
        
        it getUserData(const int id);
        const_it getUserData(const int id) const;
        
        bool countData(const int id) const;
        
        it begin();
        const_it begin() const;
        it end();
        const_it end() const;

		
		SharedPtr<SpType> returnSecond(const int id); 
		

        
        int sizeData() const;
        
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
int
TotalDatabase<SpType>::addUser(SpType *newdata){
    SharedPtr<SpType> temp(newdata);
    this->UserData.insert(std::make_pair(this->userId, temp));
    int tempId = this->userId;
    ++(this->userId);
    return (tempId);
}




template <typename SpType>
int
TotalDatabase<SpType>::addUserWithId(SpType *newdata){
    SharedPtr<SpType> temp(newdata);
    this->UserData.insert(std::make_pair(this->userId, temp));
    newdata->setId(this->userId);
    int tempId = this->userId;
    ++(this->userId);
    return (tempId);
}

template <typename SpType>
int TotalDatabase<SpType>::addUserWithId(SharedPtr<SpType> obj)
{
    this->UserData.insert(std::make_pair(this->userId, obj));
    obj->setId(this->userId);          
    return this->userId++;
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
SharedPtr<SpType>
TotalDatabase<SpType>::returnSecond(const int id){
	TotalDatabase<SpType>::it its = this->UserData.find(id);
    if (its == this->UserData.end()) {
        return SharedPtr<SpType>(); 
    }
	return(its->second);
}

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