# ft_irc

Make to irc server

[확인해야하는 작업로그](./Doc/ModifyDoc.MD)

채널이 하나 만들어졌고, 그 안에서 읽고 쓰는거

다중채널, 유저들 정보 받아와서 데이터베이스(키와 벨류-> map 값으로 스마트포인터로 관리.), 파싱 후 명령어 처리하기, 시그널, tcp/ip 통신?


읽고 쓰는거


스마트 포인터로 데이터베이스 자체의 값들 관리할 수 있게되었고,
채널 같은 경우도 가장 첫번째가 로비
다음 채널부터 채팅을 할 수 있는 가상공간.


스마트 포인터를 어디에 사용하는게 좋을까에 대한 생각

1. 서버 전체 유저 관리

std::map<int, SharedPtr<User>> (or allUsers)

2. 채널별 유저 명단(ChannelData/TotalDatabase)

ChannelData에서 **SharedPtr<User> who;**로 저장

TotalDatabase<ChannelData>에서 ChannelData(SharedPtr<User>) 사용

서버와 채널 모두 스마트 포인터로 관리해야 꼬일 걱정이 없음.


---

6월 14일

해야할거

1. 혼자 다중으로 같은 채널 많이 들어가지는거 없애기 (완)

```cpp
if (channel->hasUserById(user.getId())) {
            user.numeric(ERR_FATAL,
            channelName + " :is already on channel");
            continue;
        }
hasUserById() <- CD 유저 데이터 잆어올 수 있는 함수 itorator로 작동함니다.
```

---

2. kick, 전부 자기자신 나가지는거 고쳐야하고, 로비 만들어지고 채널 유저 목록을 스캔 한 사람이라도 auth >= 7(op 플래그)이 있으면 → 이미 오퍼레이터가 있으므로 아무 것도 하지 않고 리턴.  
op 가 전혀 없을 때만 채널에 아직 남아 있는 첫 번째(or 임의) 유저를 골라 setAuth(7) 으로 오퍼레이터 권한을 부여.

(선택) MODE +o <nick> 브로드캐스트로 모두에게 알림.

> #### 단계 설명
> 
> 1. `파라미터 ≥ 2` 확인 : 없으면 `461`	
> 2. 채널 존재 검사 : 없으면 `403`	
> 3. 호출자 op 권한 확인 : op 아니면 `482`	
> 4. modeStr 분기 : if/else 체인으로 각 플래그별 작업 수행	
> 5. 성공 시 브로드캐스트 + 호출자에게도 회신
> 
> `:<nick>!user@host MODE #chan <modestring> [arg]\r\n`


sharedptr로 acceptclient안에 수정.

<details>
<summary> 함수 추가 </summary>

```cpp
void Server::applyOpFlag(Channel* ch,
                         const std::string& nick,
                         bool give,             // true = +o, false = -o
                         User& src)
{
    User* tgt = getUserByNick(nick);
    if (!tgt || !ch->hasUserById(tgt->getId())) {
        src.numeric(441, nick + " " + ch->getChannelName() +
                          " :They aren't on that channel");
        return;
    }
    SharedPtr<ChannelData> cd =
        ch->getChannelUsers().getUserData(tgt->getId())->second;
    cd->setAuth(give ? 7 : 0);

    /* MODE echo (op 변경은 곧바로 채널에도 전파) */
    std::string m = ":" + src.fullPrefix() + " MODE " +
                    ch->getChannelName() + (give?" +o ":" -o ") + nick + "\r\n";
    ch->broadcast(m, nullptr);
}
```
</details>

---

3. quit 안나가지는 버그(완) 2025년 6월 23일
quit을 고쳤다.
    // 6. User 비활성화 표시 (poll 루프에서 소켓 close 조건으로 사용)
    for (size_t i = 1; i < _pfds.size(); ++i)
    {
    if (_pfds[i].fd == user.getFd()) {
        _disconnectUser(i);      // FD close + poll erase + users.erase()
        break;
    }

마지막에 이렇게 quit을 비활성화만 시키는 것이 아니라, fd close + poll 지워줘야 다 나가진다.

4. 채널 비밀번호 쳐서 들가지게 지금은 그냥 들어가짐(완) 2025년 6월 23일
어떻게 고쳤냐면
서버가 만들어질때 비번이 있는지 없는지를 확인했다.
            if (i < channelKeys.size() && Utils::is_key(channelKeys[i]))
                channel->setPwdset(true, channelKeys[i]);
이걸 추가해서 확인했다.

그 다음으로, 채널에 들어가는 유저들 키 검사하는 방법을 바꿨다.
        // 3. 패스워드(키) 검사
        if (channel->getPwdSet()) {
            std::string pass = (i < channelKeys.size()) ? channelKeys[i] : "";
        
            /* 1) key 가 없으면 바로 거절 */
            if (pass.empty()) {
                user.numeric(475, channelName + " :Cannot join channel (+k)"); // ERR_BADCHANNELKEY
                continue;
            }
            /* 2) 형식 검사 */
            if (!Utils::is_key(pass)) {
                user.numeric(467, channelName + " :Bad key format");           // RFC: 467
                continue;
            }
            /* 3) 일치 여부 */
            if (channel->checkPassword(pass) == false) {
                user.numeric(475, channelName + " :Wrong key");                // Same 475
                continue;
            }
        }

마지막에 일치 여부를 확인하는 부분에서, bool값이랑 string을 비교해할 수도 있는 절체절명의 상활에서 새로운 함수를 만들어서 고쳤다.
bool Channel::checkPassword(std::string pwd){
    return (this->pwd.CheckPassword(pwd));
}
이걸 만들었는데, 어떻게 보면 getter랑 다른게 없는 느낌.
pwd의 get을 가져오는 느낌인데, bool값을 get하는 느낌으로 쓰인 것이다.
왜 패쓰워드로 바로 안하고 이렇게 했냐고 생각하면, channel의 pwd를 바로 쓸게 없었기 때문!