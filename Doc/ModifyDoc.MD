# 작업에 대한 로그를 기록하는 문서입니다.

### __이문서는 작업에 대한 일관된 Log를 남기기 위해서 작성되었습니다.__

---

<details>
<summary> 
예시 
</summary>

### 예시
> ## 수정 로그
> 
> | 날짜 | 시간 | 파일명 | 타입 | 설명 |
> |:-----|:-----|:-------|:-----|:-----|
> | 2025-06-12 | 14:30 | server.cpp | 버그수정 | [클라이언트 연결 시 오류 해결](#2025-06-12-클라이언트-연결-오류-수정) |
> | 2025-06-12 | 15:45 | channel.cpp | 기능추가 | 새로운 채널 관리 기능 구현 |
> 
> ---
> 
> ## 상세 내역
> 
> ### 2025-06-12 클라이언트 연결 오류 수정
> - **파일**: `server.cpp`
> - **라인**: 142-150
> - **문제**: 소켓 연결 시 타임아웃 발생
> - **해결**: 연결 대기 시간 조정 및 예외 처리 추가
> 
> ### 2025-06-12 채널 관리 기능 추가
> - **파일**: `channel.cpp`
> - **라인**: 새로 추가
> - **내용**: JOIN, PART, KICK 명령어 구현
> - **참고**: IRC RFC 2812 표준 준수  

> 마크다운에서 **내부 링크(앵커)**를 사용하면 됩니다. 설명 부분을 링크로 만들고 상세 내역의 제목에 앵커를 추가하세요:
>링크 규칙:
> - 링크텍스트
> - 앵커 이름은 소문자 + 하이픈으로 변환
> - 공백은 -로 변경
> - 특수문자 제거

</details>

---

## 파일  로그

> | 날짜 | 시간 | 파일명 | 타입 | 설명 |  진행사항 |
> |:-----|:-----|:-------|:-----|:-----|:-----|
> | 2025-06-12 | 14:30 | server.cpp | 무한루프 | [Server에서 무한루프후 CPU 점유율 100%달성](#무한루프-원인-파악-수정) | 완 |
> | 2025-06-12 | 15:00 | server.cpp | 개선방안 | [명령어별 개선방안](정교한-명령어를-만들기위한-방법) | 완 |
> | 2025-06-12 | 15:00 | server.cpp | segementerror | [User가 나가면 프로그램이 segmentError 발생](#User가-나갈시-segmenterror-발생) | 완 |
> | 2025-06-12 | 16:51 | server.cpp | index불일치 | [_pfds의 index와 _user의 index불일치](#_pfds의-index와-_user의-index불일치) | 진 |
> | 2025-06-12 | 17:30 | server.hpp | 변수삭제 | readflag를 없앴습니다 상세내용은 없습니다. | 완 |

## 상세 내역

### 무한루프 원인 파악 수정
- **파일** : [server.cpp](../srcs/server.cpp)
- **함수** : `void Server::run()`
- **문제** : 함수의 내부 while 문에서 i 가 if문에 들어가 있어서 증가하지 못하고 계속해서 _user.getUserData(i-1)->second를 호출
- **해결** : if 문을 개별적으로 괄호로 감싸준 다음 i를 while의 조건안에서 증가하도록 만들었음.
- **참고자료** 
    - [callgrind](./analyze/call)
    - [참고 블로그](https://m.blog.naver.com/kimyoseob/220639317811)
    - [사용법](./analyze/HowToUseCallgrind.txt)

### 정교한 명령어를 만들기위한 방법
- **파일** : [server.cpp](../srcs/server.cpp)
- **함수** : `Command`
- **현재 방법** : 명령어 함수들의 인자값을 검사를 하는 과정에서 특정조건을 일일이 검사를 하고 있습니다.
- **개선 방안** : 명령어들의 형식을 검사할때 [Utils.hpp](../include/Utils.hpp) 안에있는 is_Function들을 활용해서 만들면 더욱더 정교한 명령어가 될것 같습니다.
- **참고자료** 
    - [Utils.hpp](../include/Utils.hpp)
    - [RFC2812](https://translate.autocrypt.io/rfc2812)
    - [ParsingRule](./IRC_ABNF.MD) : ASMR : ft_irc

### User가 나갈시 segmenterror 발생
- **파일** : [server.cpp](../srcs/server.cpp)
- **함수** : `void Server::_disconnectUser(size_t idx)`
- **ErrorInfo** : [1]    1392247 segmentation fault (core dumped)
- **부적절한 형식** : cpp 98버전에서는 nullptr은 허용되지 않습니다.
- **발생한 문제 파악** : User가 나가게 되면 현재가지고 있는 table의 나간 User의 정보를 잃게 된다. 우리가 처음 설계 한 모습은 모든 유저의 정보를 저장하고 User:Class안의 active의 member를 확인해서 구현을 해야하는데 유저가 나가게되고 사용할 수 없는 메모리의 주소를 참조하게 되면서 Error가 발생하게 된다.
- **해결** : 위 함수에서 user의 Data를 전부 지우지 않고 setActive와 setFd를 조정합니다. 이로써 메모리는 언제나 유효함으로 Core dumped를 없앴습니다.
- **새로운룰** : 유저가 생성이되면 fd를 -1 을 가지고 user가 나가게 되면 fd 는 -2를 가지게 됩니다.
- **참고자료** 
    - [valgrind_log](./analyze/2025-06-12/valgrind_log_2025_06_21_1531.txt)
    - [lldb_log](./analyze/2025-06-12/lldb_log_2025_06_12_1544.txt)

### _pfds의 index와 _user의 index불일치
- **파일** : [server.cpp](../srcs/server.cpp)
- **함수** : `void Server::run`
- **문제점** : 동일한 fd값의 _user, _pfds 의 index를 지정해줘야 하는데 i - 1 은 그 조건을 달성하지 못하고 있습니다. 예를 들어서 초기에는 User가 i - 1 의 index 를 가지고 있지만 나중에 user가 나가고 들어오고를 반복하게 되면 fd는 빈자리부터 할당이 됩니다. 첫번째 유저가 나가고 그렇게 되면 10번째로 들어 오게된 user _users[10] 은 _pfds[1]인데 `i - 1`의 형식을 따른다면 문제가 발생하게 됩니다.
- **해결방안** : 지금 lldb로 디버깅을 했을때 user와 _pfds의 같은 값을 찾아내면 됩니다. 둘의 같은 값은 fd 값이므로 fd 같으므로 같은 fd 값을 가진 친구를 반환 하도록 합시다.
- **추가함수** Server::getSamefdUser : _pfds의 fd 값을 _user의 fd 값이랑 비교해서 반환 하는 함수입니다. user의 active(기본 세팅을 했는지:활성화 상태인지)는 확인 하지 않습니다. 이 상태를 확인 하는건 각 명령어의 역할 이라고생각합니다.
- **참고자료** 