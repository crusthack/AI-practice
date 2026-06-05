# .NET 생태계 학습 로드맵

## 현재 디렉토리 분석

이 저장소는 .NET 입문용 단일 예제보다 조금 더 넓은 방향을 이미 갖고 있습니다. C# 콘솔 앱에서 시작해서 설정 시스템, Generic Host, Hosted Service 생명주기, 로깅, xUnit 테스트, BenchmarkDotNet, F# 라이브러리와 C# 호출까지 다룹니다.

현재 구성은 다음과 같습니다.

| 영역 | 프로젝트 | 현재 상태 | 다음 학습 포인트 |
| --- | --- | --- | --- |
| C# 및 설정 | `1. CSharpBasics` | 콘솔 앱, `appsettings.json`, 환경별 설정, 환경 변수, 명령줄 설정 | C# 문법, 타입 시스템, nullable, options pattern |
| Host/DI | `2. HostBuilder` | `Host.CreateApplicationBuilder`, Hosted Service, 앱 생명주기 로그 | DI lifetime, background worker, graceful shutdown |
| Logging | `3. Logger` | 현재는 기본 Hello World | `ILogger`, log level, structured logging, provider |
| 성능 측정 | `Benchmark` | 문자열 결합, 컬렉션 탐색, 박싱, 검색 벤치마크 | allocation, GC, Span, LINQ 비용, Release 빌드 |
| 테스트 | `Tests` | xUnit 기본 테스트와 프로젝트 참조 | AAA 패턴, Theory, fixture, mocking, coverage |
| 함수형/상호운용 | `functional` | C# benchmark에서 F# 모듈 호출 | F# 타입 노출, C#/F# interop, 함수형 pipeline 비용 |

주의할 점도 있습니다.

- README 계열 파일의 한글이 PowerShell 출력에서 깨져 보입니다. 문서 파일 인코딩을 UTF-8로 통일하는 것이 좋습니다.
- 대부분의 프로젝트가 `net10.0`을 대상으로 합니다. 학습 환경의 SDK가 .NET 10 SDK와 맞는지 먼저 확인해야 합니다.
- `3. Logger` 프로젝트는 학습 주제 이름에 비해 아직 구현이 비어 있습니다.
- `Benchmark/REAMD.md`는 파일명이 `README.md`가 아니라 `REAMD.md`입니다.

## 전체 학습 방향

목표는 단순히 C# 문법을 외우는 것이 아니라, .NET 애플리케이션이 실행되고 구성되고 관측되고 테스트되고 배포되는 흐름을 이해하는 것입니다.

권장 순서는 다음과 같습니다.

1. C# 언어와 .NET 런타임의 기본 모델을 잡는다.
2. 프로젝트 파일, CLI, 빌드, 패키지, 솔루션 구조를 익힌다.
3. Configuration, Options, Dependency Injection, Logging을 하나의 애플리케이션 골격으로 묶는다.
4. 테스트와 벤치마크로 동작과 성능을 검증한다.
5. ASP.NET Core, EF Core, background service, 배포/관측성으로 확장한다.
6. 고급 주제로 GC, async, Span, trimming, AOT, F# interop을 학습한다.

## 학습 로드맵

### 0단계. 환경과 도구

학습 목표:

- `dotnet` CLI의 역할 이해
- solution, project, package, target framework 구분
- VS Code 또는 Visual Studio에서 빌드/실행/디버깅

실습:

- `dotnet --info`
- `dotnet build`
- `dotnet run --project "1. CSharpBasics"`
- `dotnet test`
- `dotnet sln list`

성과물:

- 저장소 루트 README에 실행 방법 정리
- 각 프로젝트별 목적 한 줄 설명 추가

### 1단계. C# 기본기

학습 목표:

- 값 타입과 참조 타입
- class, struct, record
- nullable reference type
- property, field, method
- collection, generic, LINQ
- exception 처리

현재 연결 프로젝트:

- `1. CSharpBasics`
- `Tests`
- `Benchmark`

실습:

- `Mod.fn()`처럼 단순 함수가 아니라 입력을 받는 순수 함수 작성
- xUnit으로 정상/예외/경계값 테스트 작성
- `List<T>`, `Dictionary<TKey,TValue>`, `HashSet<T>` 성능 비교

목차:

1. C# 프로그램 구조
2. 네임스페이스와 접근 제한자
3. 기본 타입과 nullable
4. class, struct, record
5. interface와 abstract class
6. generic
7. collection과 LINQ
8. exception과 result-style 처리
9. pattern matching
10. 파일/문자열/날짜 처리

### 2단계. 프로젝트와 빌드 시스템

학습 목표:

- `.csproj` 구조 이해
- `TargetFramework`, `ImplicitUsings`, `Nullable`
- `PackageReference`, `ProjectReference`
- Debug/Release 차이
- launch profile

현재 연결 프로젝트:

- 모든 `.csproj`
- `Properties/launchSettings.json`

실습:

- 새 class library 프로젝트 추가
- console app에서 library 참조
- Debug/Release 빌드 결과 비교
- `appsettings*.json` output copy 동작 확인

목차:

1. SDK-style project
2. solution과 project
3. target framework
4. NuGet package
5. project reference
6. build configuration
7. launchSettings
8. output directory와 content file

### 3단계. Configuration과 Options

학습 목표:

- 설정 provider 우선순위 이해
- `appsettings.json`, 환경별 JSON, 환경 변수, command line
- `IConfiguration` 직접 접근과 Options Pattern 차이
- 설정 validation

현재 연결 프로젝트:

- `1. CSharpBasics`

실습:

- `CustomOptions` class 작성
- `IOptions<T>`, `IOptionsSnapshot<T>`, `IOptionsMonitor<T>` 비교
- 환경별 설정 override 확인

목차:

1. Configuration provider
2. JSON configuration
3. environment variable
4. command line argument
5. provider 우선순위
6. strongly typed options
7. options validation
8. secret 관리 기본

### 4단계. Generic Host와 Dependency Injection

학습 목표:

- Generic Host가 무엇을 제공하는지 이해
- DI container 기본 사용
- singleton, scoped, transient lifetime
- hosted service와 application lifetime
- graceful shutdown

현재 연결 프로젝트:

- `2. HostBuilder`

실습:

- `IClock`, `IMessageWriter` 같은 service interface 추가
- lifetime별 객체 생성 시점 로그 출력
- `BackgroundService` 기반 worker 구현
- cancellation token 처리

목차:

1. Generic Host 개념
2. `HostApplicationBuilder`
3. service registration
4. DI lifetime
5. constructor injection
6. `IHostedService`
7. `BackgroundService`
8. application lifetime event
9. graceful shutdown

### 5단계. Logging과 관측성

학습 목표:

- `ILogger<T>` 사용
- log level과 category
- structured logging
- console/debug/event source provider
- trace, metric, OpenTelemetry 입문

현재 연결 프로젝트:

- `3. Logger`
- `2. HostBuilder`

실습:

- `3. Logger`를 실제 logging 예제로 확장
- log level을 appsettings로 제어
- scope와 structured property 출력
- 예외 로그와 일반 문자열 로그 비교

목차:

1. logging abstraction
2. log level
3. category
4. structured logging
5. logging provider
6. logging configuration
7. exception logging
8. scope
9. OpenTelemetry 기초

### 6단계. 테스트

학습 목표:

- xUnit 구조
- unit test와 integration test 구분
- AAA 패턴
- Theory와 InlineData
- fixture
- test double과 mocking
- coverage

현재 연결 프로젝트:

- `Tests`
- `1. CSharpBasics`

실습:

- `Mod`를 계산/파싱/검증 예제로 확장
- 정상 케이스와 실패 케이스 테스트
- `Theory`로 입력 테이블 기반 테스트 작성
- configuration을 사용하는 class 테스트

목차:

1. 테스트 프로젝트 구조
2. Fact와 Theory
3. AAA 패턴
4. assertion
5. exception test
6. fixture
7. mocking
8. integration test
9. coverage

### 7단계. 성능과 메모리

학습 목표:

- BenchmarkDotNet 사용법
- Release 빌드의 중요성
- allocation과 GC
- LINQ와 loop 비용
- boxing
- collection 선택 기준
- `Span<T>`와 `Memory<T>` 입문

현재 연결 프로젝트:

- `Benchmark`
- `functional`

실습:

- `string` vs `StringBuilder`
- `List.Contains` vs `HashSet.Contains`
- LINQ pipeline vs imperative loop
- boxing allocation 측정
- `Span<char>` 기반 문자열 처리 예제 추가

목차:

1. BenchmarkDotNet 기본
2. benchmark job과 diagnoser
3. 평균, 표준편차, allocation 해석
4. Release 빌드
5. GC generation
6. boxing과 unboxing
7. LINQ 비용
8. collection 성능
9. Span과 Memory
10. profiling 기본

### 8단계. ASP.NET Core

학습 목표:

- HTTP 요청/응답 모델
- minimal API
- controller
- middleware
- routing
- model binding과 validation
- dependency injection
- configuration/logging 연동

현재 저장소에는 아직 ASP.NET Core 프로젝트가 없습니다. 다음 단계에서 `4. WebApi` 프로젝트로 추가하는 것이 좋습니다.

실습:

- `dotnet new webapi -o "4. WebApi"`
- health check endpoint
- CRUD minimal API
- validation
- integration test

목차:

1. ASP.NET Core hosting
2. minimal API
3. routing
4. middleware
5. controller
6. model binding
7. validation
8. error handling
9. Swagger/OpenAPI
10. integration test

### 9단계. 데이터 접근과 EF Core

학습 목표:

- ORM의 역할
- DbContext
- entity mapping
- migration
- query tracking
- transaction
- repository 패턴의 장단점

권장 신규 프로젝트:

- `5. EfCore`

실습:

- SQLite 기반 Todo API
- migration 생성/적용
- LINQ query SQL 확인
- tracking/no-tracking 비교

목차:

1. EF Core 개요
2. DbContext
3. entity와 value object
4. relationship
5. migration
6. LINQ query
7. tracking
8. transaction
9. concurrency
10. 성능 튜닝

### 10단계. 비동기와 동시성

학습 목표:

- `Task`, `async`, `await`
- cancellation token
- thread pool
- parallelism과 concurrency 차이
- channel, timer, background queue

현재 연결 프로젝트:

- `2. HostBuilder`
- 향후 `4. WebApi`

실습:

- `BackgroundService`에서 주기 작업 구현
- cancellation token 미처리/처리 비교
- `Task.WhenAll`과 순차 실행 비교
- `Channel<T>` 기반 작업 큐 구현

목차:

1. Task
2. async/await
3. cancellation
4. exception propagation
5. thread pool
6. synchronization context
7. parallel loop
8. channel
9. background queue

### 11단계. F#과 .NET 언어 상호운용

학습 목표:

- C#에서 F# 라이브러리 호출
- F# record, class, struct 노출 방식
- 함수형 pipeline과 allocation
- 도메인 로직을 F#으로 분리하는 기준

현재 연결 프로젝트:

- `functional`
- `functional/module`

실습:

- F# record를 C#에서 사용
- C# DTO와 F# 함수형 로직 연결
- F# single-pass 함수와 C# loop benchmark 비교

목차:

1. F# 프로젝트 구조
2. module과 namespace
3. F# function export
4. record/class/struct interop
5. C#에서 F# 호출
6. 함수형 pipeline 비용
7. 도메인 모델링

### 12단계. 배포와 운영

학습 목표:

- publish
- self-contained vs framework-dependent
- single file
- trimming
- container
- configuration by environment
- health check와 metrics

실습:

- `dotnet publish`
- single-file publish
- Dockerfile 작성
- 환경 변수로 설정 주입
- 로그와 health endpoint 확인

목차:

1. publish
2. runtime identifier
3. framework-dependent deployment
4. self-contained deployment
5. single file
6. trimming
7. Native AOT 개요
8. Docker
9. environment configuration
10. 운영 로그와 metric

## 추천 저장소 목차

향후 이 저장소를 다음처럼 확장하면 학습 흐름이 자연스럽습니다.

```text
practice-dotnet/
  README.md
  DOTNET_LEARNING_ROADMAP.md
  1. CSharpBasics/
  2. HostBuilder/
  3. Logger/
  4. WebApi/
  5. EfCore/
  6. AsyncConcurrency/
  7. Observability/
  Benchmark/
  Tests/
  functional/
```

## 우선 작업 순서

1. README 인코딩을 UTF-8로 정리한다.
2. `3. Logger`를 실제 로깅 예제로 구현한다.
3. `1. CSharpBasics`에 C# 타입/컬렉션/nullable 예제를 추가한다.
4. `Tests`에 `Theory` 기반 테스트를 추가한다.
5. `Benchmark/REAMD.md`를 `Benchmark/README.md`로 정리한다.
6. `4. WebApi` 프로젝트를 추가한다.
7. `5. EfCore` 프로젝트를 추가한다.
8. Host, logging, configuration, test, benchmark를 Web API 프로젝트에 통합한다.

## 8주 학습 플랜

| 주차 | 주제 | 실습 산출물 |
| --- | --- | --- |
| 1주차 | CLI, csproj, C# 기본 | `1. CSharpBasics` 문법 예제와 테스트 |
| 2주차 | configuration/options | 환경별 설정과 options validation |
| 3주차 | DI와 Generic Host | worker service와 lifetime 로그 |
| 4주차 | logging | structured logging 예제 |
| 5주차 | testing | xUnit theory, fixture, coverage |
| 6주차 | benchmarking | collection, LINQ, allocation 비교 |
| 7주차 | ASP.NET Core | minimal API와 integration test |
| 8주차 | EF Core/운영 | SQLite CRUD, publish, health check |

## 최종 목표 프로젝트

로드맵의 마지막에는 작은 운영형 API 하나를 완성하는 것이 좋습니다.

예시: `LearningTracker.Api`

기능:

- 학습 항목 CRUD
- 학습 완료 체크
- 태그/카테고리
- SQLite 저장
- configuration/options 사용
- structured logging
- xUnit integration test
- BenchmarkDotNet으로 일부 로직 성능 비교
- `dotnet publish`로 배포 산출물 생성

이 프로젝트 하나에 .NET 생태계의 핵심인 CLI, C#, configuration, DI, logging, test, benchmark, web, data, publish가 모두 들어갑니다.

## 30개 대목차 기반 장기 로드맵

아래 목차는 이 저장소를 장기 학습용 .NET 워크북으로 확장한다고 가정한 대용량 로드맵입니다. 앞쪽은 C#과 .NET 런타임의 기본기를 다지고, 중간은 애플리케이션 개발에 필요한 ASP.NET Core/EF Core/테스트를 다루며, 뒤쪽은 성능, 운영, 아키텍처, 실전 프로젝트로 이어집니다.

### 1. .NET 생태계 개요

- .NET, .NET Framework, .NET Standard 차이
- CLR, BCL, SDK, runtime
- C#, F#, VB.NET의 위치
- LTS/STS 릴리스 모델
- .NET이 적합한 애플리케이션 유형

### 2. 개발 환경과 CLI

- SDK 설치와 `dotnet --info`
- `dotnet new`, `build`, `run`, `test`, `publish`
- solution과 project 관리
- VS Code, Visual Studio, Rider 비교
- launch profile과 디버깅

### 3. 프로젝트 파일과 빌드 시스템

- SDK-style `.csproj`
- `TargetFramework`
- `ImplicitUsings`
- `Nullable`
- `PackageReference`
- `ProjectReference`
- Debug/Release configuration
- MSBuild 기본 개념

### 4. C# 프로그램 구조

- top-level statement
- namespace
- class와 member
- 접근 제한자
- entry point
- assembly와 namespace의 차이

### 5. C# 타입 시스템

- 값 타입과 참조 타입
- primitive type
- enum
- nullable value type
- nullable reference type
- boxing/unboxing
- type conversion

### 6. 객체지향 프로그래밍

- class
- struct
- record
- interface
- abstract class
- inheritance
- composition
- polymorphism

### 7. 메서드와 제네릭

- method signature
- optional parameter
- named argument
- `ref`, `out`, `in`
- generic type
- generic method
- generic constraint

### 8. 컬렉션과 LINQ

- array
- `List<T>`
- `Dictionary<TKey,TValue>`
- `HashSet<T>`
- `Queue<T>`, `Stack<T>`
- `IEnumerable<T>`
- LINQ query/operator
- deferred execution

### 9. 예외와 에러 처리

- exception hierarchy
- `try/catch/finally`
- custom exception
- exception filter
- fail-fast와 graceful handling
- Result 스타일 처리
- logging과 exception 연결

### 10. 파일, 문자열, 날짜 처리

- `StringBuilder`
- string interpolation
- encoding
- file IO
- path 처리
- `DateTime`, `DateTimeOffset`
- time zone
- JSON serialization

### 11. Configuration 시스템

- `IConfiguration`
- JSON provider
- environment variable provider
- command line provider
- provider 우선순위
- 환경별 `appsettings`
- secret 관리

### 12. Options Pattern

- strongly typed options
- `IOptions<T>`
- `IOptionsSnapshot<T>`
- `IOptionsMonitor<T>`
- options validation
- reload on change
- configuration과 DI 연결

### 13. Dependency Injection

- DI container 기본
- constructor injection
- service registration
- singleton
- scoped
- transient
- service disposal
- lifetime mismatch

### 14. Generic Host

- `HostApplicationBuilder`
- `IHost`
- host lifecycle
- `IHostedService`
- `BackgroundService`
- cancellation token
- graceful shutdown
- worker service

### 15. Logging

- `ILogger<T>`
- log level
- category
- structured logging
- log scope
- provider
- configuration 기반 log level 제어
- exception logging

### 16. 테스트 기초

- xUnit 구조
- `Fact`
- `Theory`
- AAA 패턴
- assertion
- test naming
- 테스트 가능한 코드 설계
- project reference

### 17. 테스트 심화

- fixture
- mocking
- test double
- integration test
- coverage
- snapshot test 개념
- flaky test 방지
- CI에서 테스트 실행

### 18. BenchmarkDotNet과 성능 측정

- benchmark 작성법
- job
- diagnoser
- memory diagnoser
- warmup과 iteration
- Release 빌드
- 결과 표 해석
- microbenchmark의 함정

### 19. 메모리와 GC

- managed memory
- stack과 heap
- allocation
- GC generation
- LOH
- finalizer
- `IDisposable`
- object pooling

### 20. 고성능 C#

- `Span<T>`
- `Memory<T>`
- `ReadOnlySpan<T>`
- allocation 줄이기
- LINQ 비용
- delegate allocation
- struct 사용 기준
- `ArrayPool<T>`

### 21. 비동기 프로그래밍

- `Task`
- `async/await`
- `ValueTask`
- cancellation
- timeout
- exception propagation
- `Task.WhenAll`
- async stream

### 22. 동시성과 병렬성

- thread와 thread pool
- concurrency vs parallelism
- `Parallel.ForEach`
- lock
- `SemaphoreSlim`
- channel
- producer/consumer
- background queue

### 23. ASP.NET Core 기초

- web host
- HTTP request/response
- routing
- middleware
- minimal API
- controller
- Swagger/OpenAPI
- dependency injection 통합

### 24. ASP.NET Core 애플리케이션 설계

- request pipeline
- model binding
- validation
- error handling
- filter
- endpoint group
- versioning
- health check

### 25. 인증과 인가

- authentication vs authorization
- cookie authentication
- JWT bearer
- claims
- policy
- role
- OAuth/OIDC 개요
- API 보안 기본

### 26. EF Core 기초

- ORM 개념
- DbContext
- DbSet
- entity
- relationship
- migration
- SQLite/SQL Server 연결
- LINQ query

### 27. EF Core 심화

- tracking/no-tracking
- eager/lazy/explicit loading
- transaction
- concurrency token
- query performance
- compiled query
- raw SQL
- migration 운영 전략

### 28. 아키텍처와 설계 패턴

- layered architecture
- clean architecture
- vertical slice
- domain model
- DTO와 entity 분리
- repository 패턴 검토
- CQRS 개요
- dependency boundary

### 29. 운영, 배포, 관측성

- `dotnet publish`
- framework-dependent deployment
- self-contained deployment
- single file
- trimming
- Native AOT 개요
- Docker
- OpenTelemetry
- metrics, tracing, logging

### 30. 종합 실전 프로젝트

- `LearningTracker.Api` 설계
- minimal API 또는 controller 선택
- EF Core 기반 CRUD
- options/configuration 적용
- structured logging 적용
- xUnit integration test
- BenchmarkDotNet으로 핵심 로직 측정
- publish와 container 배포

## 30개 목차별 권장 실습 프로젝트

| 목차 | 실습 프로젝트 | 산출물 |
| --- | --- | --- |
| 1-3 | `0. Tooling` | CLI/build/package 실습 정리 |
| 4-10 | `1. CSharpBasics` | C# 기본 문법 예제와 단위 테스트 |
| 11-12 | `1. CSharpBasics` | configuration/options 예제 |
| 13-14 | `2. HostBuilder` | DI lifetime과 worker service |
| 15 | `3. Logger` | structured logging 콘솔 앱 |
| 16-17 | `Tests` | xUnit 테스트 스위트 |
| 18-20 | `Benchmark` | 성능/메모리 벤치마크 |
| 21-22 | `6. AsyncConcurrency` | channel 기반 background queue |
| 23-25 | `4. WebApi` | 인증 포함 Web API |
| 26-27 | `5. EfCore` | SQLite/SQL Server CRUD |
| 28 | `8. Architecture` | 계층형/vertical slice 비교 |
| 29 | `7. Observability` | publish, Docker, OpenTelemetry |
| 30 | `LearningTracker.Api` | 최종 통합 프로젝트 |

## 대용량 학습 순서

이 목차를 처음부터 끝까지 선형으로만 진행할 필요는 없습니다. 추천 진행 방식은 다음과 같습니다.

1. 1-15장은 기초 체력 구간입니다. C#, 빌드, 설정, DI, Host, Logging을 먼저 연결합니다.
2. 16-20장은 검증 구간입니다. 테스트와 벤치마크로 코드의 동작과 비용을 확인합니다.
3. 21-27장은 애플리케이션 구간입니다. 비동기, Web API, EF Core를 통해 실제 서비스를 만듭니다.
4. 28-30장은 실전 구간입니다. 아키텍처, 운영, 배포, 최종 프로젝트로 마무리합니다.
