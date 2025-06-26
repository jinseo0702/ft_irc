# IRC 서버 코드 중복 패턴 분석

### 위치찾기

### 공통 패턴:
```cpp
for (TotalDatabase<TYPE>::it it = container.begin(); it != container.end(); ++it) {
    if (it->second->getAttribute() == searchValue) {
        return it->second;  // 또는 break
    }
}
```

[void Server::_disconnectUser(size_t idx)->Server.cpp:170](../../../srcs/Server.cpp) : TotalDataBase< User> 의 fd 와 주어진 fd 와 같은 값 찾기

[void Server::handleJoin(User& user, const Parser& parser)->Server.cpp:379](../../../srcs/Server.cpp) : TotalDataBase< Channel> 의 이름과 채널이름과 같은 값 찾기


[void Server::handleNick(User& user, const Parser& parser)->Server.cpp:473](../../../srcs/Server.cpp) : TotalDataBase< User> 의 이름과 유저이름과 같은 값 찾기

[void Server::handleNick(User& user, const Parser& parser)->Server.cpp:494](../../../srcs/Server.cpp) : TotalDataBase< User> 의 해당채널에 유저가 있는지 검사

[void Server::handleQuit(User& user, const Parser& parser)->Server.cpp:613](../../../srcs/Server.cpp) : TotalDataBase< Channel> 전체 _channels 순회 후 유저 제거

[void Server::handleKick(User& user, const Parser& parser)->Server.cpp:807](../../../srcs/Server.cpp) : TotalDataBase< ChannelData> ChannelData의 이름과 주어진 이름과 같은지 검사

[void Server::handleShow(User& u, const Parser& p)->Server.cpp:1113](../../../srcs/Server.cpp) : TotalDataBase< Channel> Channel의 이름과 주어진 이름과 같은지 검사

[void Server::handleShow(User& u, const Parser& p)->Server.cpp:1113](../../../srcs/Server.cpp) : TotalDatabase< ChannelData>를 전체 순회하면서 출력하기

[Channel* Server::getChannelByName(const std::string& name)->Server.cpp:1149](../../../srcs/Server.cpp) : TotalDatabase< Channel> 주어진 채널이름이 데이터 베이스 안에있는지 확인

[User* Server::getUserByNick(const std::string& nick->Server.cpp:1156](../../../srcs/Server.cpp) : TotalDatabase< User> 주어진 유저이름이 데이터 베이스 안에있는지 확인

[int    Server::getSamefdUser(const int _pfdsFd)->Server.cpp:1165](../../../srcs/Server.cpp) : TotalDatabase< User> 주어진 유저fd가 데이터 베이스 안에있는지 확인


---
> 어떻게 수정하면될까?  
> 반복되는 패턴을 찾아서 TotalDatabase안에서 자동으로 찾도록 만든다.  
> 패턴을 찾아보자   
> 지금 나오는 패턴은 





