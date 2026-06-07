# 프롬프트: 특정 Phase 전체 구현 (뼈대 → 동작 코드)

> 이 파일 전체를 Claude Code에 붙여넣어 사용합니다.  
> `[변수 입력]` 섹션만 실제 값으로 채운 뒤 전달하세요.

---

## [변수 입력]

```
TARGET_PRACTICE = practice-Network           # 대상 디렉토리
TARGET_PHASE    = Phase1_SocketBasics        # 구현할 Phase 폴더
LANGUAGE        = cpp                        # cpp 또는 dotnet

# 이 Phase에서 구현할 모듈 목록 (LEARNING_GUIDE에서 확인)
MODULES:
  01_TCP_Server
  02_TCP_Client
  03_UDP_Echo
  04_Socket_Options

# 구현 우선순위 (high = 꼭 동작해야 함, low = 개념 스텁 허용)
PRIORITY = high
```

---

## 에이전트 지시사항

위 변수들을 사용해 `TARGET_PHASE`의 모든 모듈을 구현합니다.

### Phase 0: 준비

1. `TARGET_PRACTICE/CLAUDE.md` 를 읽어 코딩 컨벤션, 초기화 템플릿, 프로젝트 속성을 파악합니다.
2. `TARGET_PRACTICE/LEARNING_GUIDE.md` 를 읽어 각 모듈의 목표, 핵심 API, 확장 과제를 파악합니다.
3. `TARGET_PHASE/` 폴더 내 기존 파일을 확인합니다.

### Phase 1: 모듈별 구현

`MODULES` 목록의 각 모듈에 대해 순서대로 아래 작업을 수행합니다.

#### 1-A. 폴더 및 소스 파일 생성

모듈 폴더가 없으면 생성합니다.

**C++ 모듈:**
- `main.cpp` — 완전히 동작하는 구현
  - `CLAUDE.md` 초기화 템플릿으로 시작
  - `LEARNING_GUIDE`의 목표를 완전히 달성하는 코드
  - 핵심 API를 모두 사용
  - 실행 시 의미 있는 콘솔 출력
- `<모듈명>.vcxproj` — Visual Studio 프로젝트 파일
  - `CLAUDE.md`의 프로젝트 속성 기본값 적용

**C# 모듈:**
- `Program.cs` — 완전히 동작하는 구현
  - `CLAUDE.md` 초기화 템플릿으로 시작
  - `LEARNING_GUIDE`의 목표를 완전히 달성하는 코드
  - 실행 시 의미 있는 콘솔 출력
- `<모듈명>.csproj` — 프로젝트 파일
  - `CLAUDE.md`의 `.csproj` 기본값 적용
  - 필요한 NuGet 패키지 `<PackageReference>` 추가

#### 1-B. 빌드 확인

**C++:**
```powershell
MSBuild "<모듈경로>\<모듈명>.vcxproj" /p:Configuration=Debug /p:Platform=x64 /v:minimal
```

**C#:**
```powershell
dotnet build "<모듈경로>\<모듈명>.csproj" --verbosity minimal
```

빌드 실패 시 오류를 수정하고 다시 빌드합니다. 다음 모듈로 넘어가기 전에 빌드 성공을 확인합니다.

### Phase 2: 문서 업데이트

모든 모듈 구현 완료 후:

1. **CLAUDE.md 업데이트**: Phase별 진행 현황 표에서 `TARGET_PHASE` 상태를 `미시작` → `완료` 로 변경합니다.

2. **CATALOG.md 업데이트**: `C:\Users\crust\Documents\AI-practice\CATALOG.md` 의 `TARGET_PRACTICE` 행에서
   - 구현된 모듈 수를 업데이트합니다.
   - 모든 Phase가 완료되면 상태를 `🔵` → `✅` 로 변경합니다.

### Phase 3: 완료 보고

아래 형식으로 결과를 출력합니다.

```
=== 구현 완료: TARGET_PHASE in TARGET_PRACTICE ===

성공한 모듈:
  ✅ 01_TCP_Server     — main.cpp, vcxproj, 빌드 성공
  ✅ 02_TCP_Client     — main.cpp, vcxproj, 빌드 성공
  ...

실패하거나 스텁으로 남긴 모듈:
  ⚠️ XX_ModuleName    — 사유: (예: 관리자 권한 필요, 외부 라이브러리 없음)

다음 권고 Phase: (다음으로 학습하기 좋은 Phase 이름)
```

---

## 품질 기준

- 모든 모듈(`PRIORITY = high`) 빌드 성공
- 각 모듈 실행 시 의미 있는 출력 존재
- 핵심 API가 실제로 코드에 사용됨
- CLAUDE.md·CATALOG.md 업데이트 완료
