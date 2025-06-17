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




6월 14일

해야할거
1. 혼자 다중으로 같은 채널 많이 들어가지는거 없애기 (완)
if (channel->hasUserById(user.getId())) {
            user.numeric(ERR_FATAL,
            channelName + " :is already on channel");
            continue;
        }
hasUserById() <- CD 유저 데이터 잆어올 수 있는 함수 itorator로 작동함니다.
2. kick, 전부 자기자신 나가지는거 고쳐야하고, 로비 만들어지고 
채널 유저 목록을 스캔
한 사람이라도 auth >= 7(op 플래그)이 있으면
→ 이미 오퍼레이터가 있으므로 아무 것도 하지 않고 리턴.

op 가 전혀 없을 때만

채널에 아직 남아 있는 첫 번째(or 임의) 유저를 골라

setAuth(7) 으로 오퍼레이터 권한을 부여.

(선택) MODE +o <nick> 브로드캐스트로 모두에게 알림.
단계	설명
① 파라미터 ≥ 2 확인 : 없으면 461	
② 채널 존재 검사 : 없으면 403	
③ 호출자 op 권한 확인 : op 아니면 482	
④ modeStr 분기 : if/else 체인으로 각 플래그별 작업 수행	
⑤ 성공 시 브로드캐스트 + 호출자에게도 회신
:<nick>!user@host MODE #chan <modestring> [arg]\r\n

sharedptr로 acceptclient안에 수정.

함수 추가
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

3, 채널 비밀번호 쳐서 들가지게 지금은 그냥 들어가짐

