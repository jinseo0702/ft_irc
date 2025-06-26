# IRC 서버 코드 중복 패턴 분석

## 1. 데이터베이스 탐색 패턴

| 패턴 유형 | 출현 위치 | 반복 횟수 | 유사도 | 우선순위 |
|----------|----------|----------|--------|----------|
| **채널 검색 (이름 기준)** | `handleJoin()`, `handlePart()`, `handlePrivMsg()`, `handleNotice()`, `handleKick()`, `handleInvite()`, `handleTopic()`, `handleMode()`, `handleShow()`, `getChannelByName()` | **10회** | 95% | 🔥 최고 |
| **유저 검색 (닉네임 기준)** | `handleNick()`, `handleKick()`, `handleInvite()`, `applyOpFlag()`, `getUserByNick()` | **5회** | 90% | 🔥 최고 |
| **유저 검색 (FD 기준)** | `_disconnectUser()`, `getSamefdUser()` | **2회** | 85% | 🔴 높음 |

### 공통 패턴:
```cpp
for (TotalDatabase<TYPE>::it it = container.begin(); it != container.end(); ++it) {
    if (it->second->getAttribute() == searchValue) {
        return it->second;  // 또는 break
    }
}
```

---

## 2. 권한 검사 패턴

| 검사 유형 | 출현 위치 | 반복 횟수 | 유사도 | 우선순위 |
|----------|----------|----------|--------|----------|
| **오퍼레이터 권한 확인** | `handleKick()`, `handleInvite()`, `handleTopic()`, `handleMode()`, `applyOpFlag()` | **5회** | 95% | 🔥 최고 |
| **채널 멤버십 확인** | `handlePart()`, `handleKick()`, `handleInvite()`, `handlePrivMsg()` | **4회** | 90% | 🔴 높음 |
| **ID 변환 후 권한 확인** | 거의 모든 채널 관련 핸들러 | **8회** | 85% | 🔴 높음 |

### 공통 패턴:
```cpp
int changeId = ch->changeServerIdtoChannel(user.getId());
TotalDatabase<ChannelData>::const_it cit = ch->getChannelUsers().getUserData(changeId);
if (cit == ch->getChannelUsers().end() || cit->second->getAuth() < 7) {
    // 권한 없음 처리
}
```

---

## 3. 에러 메시지 전송 패턴

| 에러 유형 | 출현 위치 | 반복 횟수 | 유사도 | 우선순위 |
|----------|----------|----------|--------|----------|
| **매개변수 부족** | `handleJoin()`, `handlePart()`, `handleKick()`, `handleInvite()`, `handleTopic()`, `handleMode()` | **6회** | 80% | 🔴 높음 |
| **권한 없음 (482)** | `handleKick()`, `handleInvite()`, `handleTopic()`, `handleMode()` | **4회** | 95% | 🔴 높음 |
| **채널 없음 (403)** | `handlePart()`, `handleKick()`, `handleInvite()`, `handleTopic()`, `handleMode()` | **5회** | 90% | 🔴 높음 |
| **닉네임 없음 (401)** | `handleKick()`, `handleInvite()`, `handlePrivMsg()`, `applyOpFlag()` | **4회** | 90% | 🔴 높음 |

### 공통 패턴:
```cpp
user.addOutbox(":server " + errorCode + " " + user.getNickName() + " " + context + "\r\n");
```

---

## 4. 메시지 브로드캐스트 패턴

| 메시지 유형 | 출현 위치 | 반복 횟수 | 유사도 | 우선순위 |
|------------|----------|----------|--------|----------|
| **유저 접두사 생성** | `handleJoin()`, `handlePart()`, `handleQuit()`, `handleNick()`, `handleKick()`, `handlePrivMsg()`, `handleNotice()`, `handleTopic()`, `handleMode()` | **9회** | 95% | 🔥 최고 |
| **채널 브로드캐스트** | `handleJoin()`, `handlePart()`, `handleQuit()`, `handleNick()`, `handleKick()`, `handleTopic()`, `handleMode()` | **7회** | 85% | 🔴 높음 |

### 공통 패턴:
```cpp
std::string msg = ":" + user.getNickName() + "!" + user.getUserName() + "@localhost " + command + " " + params + "\r\n";
ch->broadcast(msg, excludeUser);
```

---

## 5. 매개변수 검증 패턴

| 검증 유형 | 출현 위치 | 반복 횟수 | 유사도 | 우선순위 |
|----------|----------|----------|--------|----------|
| **빈 매개변수 확인** | `handleJoin()`, `handleNick()`, `handleUser()`, `handlePart()` | **4회** | 90% | 🟡 중간 |
| **매개변수 개수 확인** | `handleNick()`, `handleUser()`, `handleKick()`, `handleInvite()`, `handleTopic()`, `handleMode()` | **6회** | 85% | 🟡 중간 |
| **형식 검증** | `handleNick()`, `handleJoin()` (키 검증), `handleMode()` | **3회** | 70% | 🟡 중간 |

---

## 6. 사용자 상태 관리 패턴

| 상태 관리 | 출현 위치 | 반복 횟수 | 유사도 | 우선순위 |
|----------|----------|----------|--------|----------|
| **신규 사용자 처리** | `handleNick()`, `handleUser()` | **2회** | 95% | 🟡 중간 |
| **활성 상태 설정** | `handleNick()`, `handleUser()` | **2회** | 90% | 🟡 중간 |
| **연결 해제 처리** | `handleQuit()`, `_disconnectUser()` | **2회** | 75% | 🟡 중간 |

---

## 7. 문자열 조작 패턴

| 조작 유형 | 출현 위치 | 반복 횟수 | 유사도 | 우선순위 |
|----------|----------|----------|--------|----------|
| **콤마 분리 파싱** | `handleJoin()` (채널명), `handlePart()`, `handlePrivMsg()`, `handleNotice()` | **4회** | 80% | 🟡 중간 |
| **trailing 파라미터 처리** | `handlePrivMsg()`, `handleNotice()`, `handleTopic()` | **3회** | 85% | 🟡 중간 |
| **여러 번의 addOutbox 호출** | `handleShow()`, `handleList()` | **2회** | 90% | 🟢 낮음 |

---

## 8. I/O 및 네트워크 패턴

| I/O 패턴 | 출현 위치 | 반복 횟수 | 유사도 | 우선순위 |
|----------|----------|----------|--------|----------|
| **POLLOUT 이벤트 설정** | `_acceptClient()`, `_readLines()` | **2회** | 85% | 🟡 중간 |
| **소켓 설정** | `_acceptClient()`, `_setupSocket()` | **2회** | 70% | 🟢 낮음 |

---

## 개선 우선순위 요약

### 🔥 최고 우선순위 (즉시 개선 필요)
1. **채널 검색 패턴** - 10회 반복, 성능에 직접 영향
2. **유저 접두사 생성** - 9회 반복, 모든 브로드캐스트에 사용
3. **오퍼레이터 권한 확인** - 5회 반복, 보안 로직

### 🔴 높은 우선순위 
1. **에러 메시지 전송 패턴** - 다양한 에러 유형에서 반복
2. **채널 브로드캐스트** - 7회 반복
3. **유저 검색 (닉네임)** - 5회 반복

### 🟡 중간 우선순위
1. **매개변수 검증** - 입력 검증 표준화
2. **문자열 파싱** - 콤마 분리 등

### 🟢 낮은 우선순위
1. **I/O 패턴** - 빈도가 낮음
2. **문자열 출력** - 성능에 미미한 영향

## 개선 효과 예상

| 패턴 개선 | 성능 향상 | 유지보수성 향상 | 코드 줄 수 감소 |
|----------|----------|---------------|---------------|
| 검색 로직 통합 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ~100줄 |
| 에러 처리 표준화 | ⭐⭐ | ⭐⭐⭐⭐⭐ | ~50줄 |
| 메시지 생성 통합 | ⭐⭐⭐ | ⭐⭐⭐⭐ | ~80줄 |
| 권한 검사 통합 | ⭐⭐ | ⭐⭐⭐⭐⭐ | ~40줄 |