# 프롬프트: 기존 practice에 새 모듈 추가

> 이 파일 전체를 Claude Code에 붙여넣어 사용합니다.  
> `[변수 입력]` 섹션만 실제 값으로 채운 뒤 전달하세요.

---

## [변수 입력]

```
TARGET_PRACTICE = practice-Network           # 추가 대상 디렉토리
TARGET_PHASE    = Phase2_IOModels            # 추가할 Phase 폴더
MODULE_NUMBER   = 05                         # 모듈 번호 (기존 번호 이어서)
MODULE_NAME     = Select_Model               # 모듈 이름 (PascalCase)
MODULE_FULL     = 05_Select_Model            # 폴더명 (번호_이름)

MODULE_GOAL     = select()로 단일 스레드에서 여러 소켓을 모니터링하는 방법을 이해합니다.
MODULE_DETAIL   = |
  TCP 에코 서버에 select()를 적용해 클라이언트 소켓 여러 개를 블로킹 없이 처리합니다.
  fd_set에 서버 소켓과 클라이언트 소켓을 등록하고, select() 반환 후 FD_ISSET으로
  이벤트가 발생한 소켓을 확인해 accept 또는 recv를 수행합니다.

LANGUAGE        = cpp                        # cpp 또는 dotnet
KEY_APIS        = select, FD_ZERO, FD_SET, FD_ISSET, FD_CLR
EXTENSION_TASK  = 타임아웃을 0으로 설정한 폴링 모드와 블로킹 모드 처리량을 비교합니다.
```

---

## 에이전트 지시사항

위 변수들을 사용해 아래 작업을 수행해주세요.

### 1. 모듈 폴더 및 소스 파일 생성

`C:\Users\crust\Documents\AI-practice\TARGET_PRACTICE\TARGET_PHASE\MODULE_FULL\` 폴더를 만들고 소스 파일을 작성합니다.

**C++ 모듈인 경우 (`LANGUAGE = cpp`):**

- `main.cpp` — 동작하는 구현 코드
  - `CLAUDE.md`의 초기화 템플릿으로 시작
  - `MODULE_GOAL`과 `MODULE_DETAIL`을 완전히 구현
  - 실행 시 관찰 가능한 결과(콘솔 출력)를 포함
  - `KEY_APIS` 전부 사용
- `<MODULE_FULL>.vcxproj` — Visual Studio 프로젝트 파일
  - `CLAUDE.md`에 정의된 프로젝트 속성 기본값 적용
  - 대상: x64, Debug/Release 구성

**C# 모듈인 경우 (`LANGUAGE = dotnet`):**

- `Program.cs` — 동작하는 구현 코드
  - `CLAUDE.md`의 초기화 템플릿으로 시작
  - `MODULE_GOAL`과 `MODULE_DETAIL`을 완전히 구현
  - 실행 시 관찰 가능한 결과(콘솔 출력)를 포함
- `<MODULE_FULL>.csproj` — .NET 프로젝트 파일
  - `CLAUDE.md`에 정의된 `.csproj` 기본값 적용

### 2. LEARNING_GUIDE.md 업데이트

`TARGET_PRACTICE\LEARNING_GUIDE.md` 를 열어 `TARGET_PHASE` 섹션에 아래 형식으로 모듈 교안을 추가합니다.

**C++ 모듈:**
```markdown
### MODULE_FULL 이름

- **목표**: MODULE_GOAL
- **관찰**: (실행 후 확인할 수 있는 동작)
- **핵심 API**: KEY_APIS
- **확장 과제**: EXTENSION_TASK
```

**C# 모듈:**
```markdown
### MODULE_FULL 이름

- **목표**: MODULE_GOAL
- **관찰**: (실행 후 확인할 수 있는 동작)
- **핵심 API / 타입**: KEY_APIS
- **확장 과제**: EXTENSION_TASK
```

### 3. CLAUDE.md 진행 현황 업데이트

`TARGET_PRACTICE\CLAUDE.md` 의 Phase별 진행 현황 표에서 해당 Phase 상태를 `미시작` → `진행중` 으로 변경합니다.

### 4. 구현 검증

**C++ 모듈:**
```powershell
MSBuild "경로\MODULE_FULL.vcxproj" /p:Configuration=Debug /p:Platform=x64
```
빌드 성공 여부를 확인합니다. 오류가 있으면 수정합니다.

**C# 모듈:**
```powershell
dotnet build "경로\MODULE_FULL.csproj"
```
빌드 성공 여부를 확인합니다. 오류가 있으면 수정합니다.

---

## 품질 기준

- 소스 파일이 오류 없이 빌드됨
- `KEY_APIS`가 모두 코드에 등장함
- 실행 시 의미 있는 콘솔 출력이 있음
- LEARNING_GUIDE에 교안 추가 완료
- CLAUDE.md 진행 현황 업데이트 완료
