# ft_irc
go to irc server



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
헷갈리지 안도록