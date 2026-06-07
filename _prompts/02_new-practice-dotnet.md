# 프롬프트: 새 C# / .NET practice 디렉토리 생성

> 이 파일 전체를 Claude Code에 붙여넣어 사용합니다.  
> `[변수 입력]` 섹션만 실제 값으로 채운 뒤 전달하세요.

---

## [변수 입력]

```
PRACTICE_NAME    = practice-DesignPatterns   # 디렉토리 이름 (practice- 접두사 포함)
TOPIC_KO         = 디자인 패턴               # 주제 한국어
TOPIC_EN         = DesignPatterns            # 주제 영어
DESCRIPTION      = GoF 23 디자인 패턴을 C#으로 구현하고 .NET 표준 라이브러리에서 어떻게 쓰이는지 확인하는 실습 모음
DOTNET_VERSION   = net8.0
PREREQUISITE     = practice-dotnet (C# 기초, OOP)
RELATED_REPOS    = (없음)

NUGET_PACKAGES:
  (없음 — 순수 C# 구현)

PHASES:
  Phase1_Creational  : 생성 패턴 (Factory, Abstract Factory, Builder, Prototype, Singleton)
  Phase2_Structural  : 구조 패턴 (Adapter, Bridge, Composite, Decorator, Facade, Flyweight, Proxy)
  Phase3_Behavioral  : 행동 패턴 (Chain, Command, Iterator, Mediator, Memento, Observer, State, Strategy, Template, Visitor)
  Phase4_RealWorld   : 실제 .NET 라이브러리에서 패턴 찾기 (IEnumerable=Iterator, LINQ=Decorator, HttpClient=Proxy 등)

MODULES_PER_PHASE:
  Phase1: FactoryMethod, AbstractFactory, Builder, Prototype, Singleton
  Phase2: Adapter, Bridge, Composite, Decorator, Facade, Flyweight, Proxy
  Phase3: ChainOfResponsibility, Command, Iterator, Mediator, Memento,
          Observer, State, Strategy, TemplateMethod, Visitor
  Phase4: DotNetIterator, LinqDecorator, HttpClientProxy, EventObserver, DependencyInjectionFactory

NOTES:
  - 각 모듈은 단독 실행 가능한 콘솔 프로그램입니다.
  - 패턴 구현 후 "패턴 없이 구현" vs "패턴 있이 구현"을 비교하는 섹션을 포함합니다.
  - Phase4는 .NET BCL 소스코드 참고 링크를 포함합니다.
```

---

## 에이전트 지시사항

위 변수들을 사용해 아래 작업을 수행해주세요.

### 1. 폴더 구조 생성

`C:\Users\crust\Documents\AI-practice\` 아래에 `PRACTICE_NAME` 폴더를 만들고, 각 Phase 폴더와 `.gitkeep` 파일을 생성합니다.

```
practice-DesignPatterns/
├── Phase1_Creational/       (.gitkeep)
├── Phase2_Structural/       (.gitkeep)
├── Phase3_Behavioral/       (.gitkeep)
├── Phase4_RealWorld/        (.gitkeep)
├── README.md
├── CLAUDE.md
├── LEARNING_GUIDE.md
└── .gitignore
```

### 2. README.md 작성

`C:\Users\crust\Documents\AI-practice\_templates\dotnet\README.md` 를 기반으로, 변수들을 채워 작성합니다.

포함 내용:
- 저장소 설명 (1~2 문장)
- 빠른 시작 (`dotnet run` 명령)
- 전체 로드맵 표 (Phase 이름 | 학습 주제)
- 모듈 목차 (Phase별 모듈 설명)
- 권장 학습 방식

### 3. CLAUDE.md 작성

`C:\Users\crust\Documents\AI-practice\_templates\dotnet\CLAUDE.md` 를 기반으로 작성합니다.

포함 내용:
- 워크스페이스 개요
- 디렉토리 구조 트리
- 새 모듈 추가 방법 (`dotnet new console` 커맨드)
- NuGet 패키지 목록
- `Program.cs` 초기화 템플릿 (주제에 맞게 조정)
- 코딩 컨벤션 (주제별 특이사항 추가)
- `.csproj` 기본값
- Phase별 진행 현황 표 (모두 `미시작`)
- 주의 사항

### 4. LEARNING_GUIDE.md 작성

모든 모듈에 대해 아래 형식으로 학습 교안을 작성합니다.

```
### <패턴 이름>

- **목표**: 이 패턴이 해결하는 문제 (What problem does it solve?)
- **구조**: 참여 클래스와 역할 (UML 없이 텍스트로 설명)
- **C# 구현 포인트**: interface, abstract class, delegate 중 어떤 것을 쓸지
- **.NET 실사용 예**: BCL이나 ASP.NET에서 이 패턴이 쓰이는 사례
- **확장 과제**: 변형 구현 또는 다른 패턴과 조합
```

### 5. .gitignore 생성

`C:\Users\crust\Documents\AI-practice\_templates\dotnet\.gitignore` 를 복사합니다.

### 6. CATALOG.md 업데이트

`C:\Users\crust\Documents\AI-practice\CATALOG.md` 에서 해당 practice 행의 상태를 `⚪` → `🔵` 로 변경하고 Phase 수와 모듈 수를 기입합니다.

---

## 품질 기준

- README·CLAUDE·LEARNING_GUIDE 세 파일 모두 작성 완료
- 모든 Phase 폴더와 .gitkeep 존재
- LEARNING_GUIDE에 모든 모듈의 학습 교안 포함
- CATALOG.md 업데이트 완료
