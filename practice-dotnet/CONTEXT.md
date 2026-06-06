# practice-dotnet — 워크스페이스 컨텍스트

> 플랫폼: .NET 10 · 언어: C# / F# · IDE: Visual Studio (PracticeDotnet.slnx)  
> 목적: .NET 플랫폼 전반을 순서대로 실습하는 학습용 솔루션

---

## 모듈 완료 현황

| # | 모듈 | 상태 | 테스트 | 벤치마크 | 비고 |
|---|------|------|--------|----------|------|
| 1 | CSharpBasics | ✅ 완료 | 12 | ✅ | |
| 2 | TypeSystem | ✅ 완료 | 29 | ✅ | |
| 3 | OOP | ✅ 완료 | 27 | ✅ | |
| 4 | Iteration | ✅ 완료 | 29 | ✅ | |
| 5 | Containers | ✅ 완료 | 27 | ✅ | |
| 6 | ExceptionHandling | ✅ 완료 | 17 | ✅ | |
| 7 | StringsAndIO | ✅ 완료 | 19 | ✅ | |
| 8 | Memory | ✅ 완료 | 21 | ✅ | |
| 9 | Async | ✅ 완료 | 13 | ✅ | |
| 10 | Concurrency | ✅ 완료 | 9 | ✅ | F# 상호운용 포함 |
| 11 | HostBuilder | ✅ 완료 | 6 | ✅ | |
| 12 | Logger | ✅ 완료 | 4 | ✅ | |
| 13 | Testing | ✅ 완료 | 29 | — | Moq, xUnit 패턴 |
| 14 | HttpClient | 🔶 진행 중 | — | — | D1~D5 작성, Program/Tests/Benchmarks 미완 |
| 15 | AspNetCore | ⬜ 미시작 | — | — | |
| 16 | EFCore | ⬜ 미시작 | — | — | |
| 17 | Architecture | ⬜ 미시작 | — | — | |
| 18 | FSharpFunctional | ✅ 완료 | — | — | F# 전용 모듈 |
| 19 | Roslyn | ✅ 완료 | 20 | ✅ | Analyzer / Generator 포함 |
| 20 | NativeAOT | ⬜ 미시작 | — | — | |
| 21 | Observability | ⬜ 미시작 | — | — | |

---

## 디렉터리 구조 (공통 패턴)

```
{N}. ModuleName/
├── {N}. ModuleName.csproj   ← OutputType: Exe (13. Testing만 Library)
├── Program.cs
├── Demos/
│   ├── Print.cs             ← Header / Section / Line 출력 헬퍼
│   ├── D1_*.cs
│   ├── D2_*.cs
│   └── ...
├── Tests/
│   ├── Tests.csproj         ← xunit 2.9.3 + Microsoft.NET.Test.Sdk 17.14.1
│   └── *Tests.cs            ← [Fact] / [Theory] 기반, 짧고 약식
└── Benchmarks/
    ├── *Bench.csproj        ← BenchmarkDotNet
    └── B1_*.cs
```

### 특수 구조 모듈

```
10. Concurrency/
└── FSharpLib/FSharpLib.fsproj   ← C#↔F# 상호운용 데모

13. Testing/
└── Domain/OrderService.cs       ← OutputType=Library (테스트 대상 도메인 코드)

19. Roslyn/
├── Analyzer/    ← DiagnosticAnalyzer
├── Generator/   ← ISourceGenerator
└── Consumer/    ← 생성 코드 소비자
```

---

## 모듈별 핵심 내용

### 1. CSharpBasics
기본 문법, 변수/타입, 제어흐름, 메서드, 배열, 문자열, LINQ 입문.

### 2. TypeSystem
| 파일 | 내용 |
|------|------|
| D1_ValueTypes | struct, enum, nullable, ref struct |
| D2_ReferenceTypes | class, interface, abstract, 참조 동등성 |
| D3_Records | record class/struct, with expression, 위치 레코드 |
| D4_Syntax | 패턴 매칭, switch expression, is/as, tuple |
| D5_Boxing | 박싱/언박싱 비용, 인터페이스 호출 오버헤드 |

**Benchmarks**: struct vs class 힙 할당, 박싱 비용, defensive copy, record equality

### 3. OOP
| 파일 | 내용 |
|------|------|
| D1_Encapsulation | 접근 제어자, 프로퍼티, init/required |
| D2_Inheritance | 가상/추상 메서드, sealed, base 호출 |
| D3_Polymorphism | 인터페이스 다형성, explicit 구현 |
| D4_Generics | 제네릭 타입 제약, variance, 공변/반변 |
| D5_DelegatesEvents | delegate, Func/Action, event, EventHandler |
| D6_Composition | 컴포지션 vs 상속, 데코레이터, 전략 패턴 |
| D7_AdvancedPatterns | 반복자, 팩토리, 빌더, 옵저버 패턴 |

### 4. Iteration
| 파일 | 내용 |
|------|------|
| D1_Loops | for/foreach/while, break/continue |
| D2_Conditionals | if/switch, 패턴 매칭, guard clause |
| D3_Collections | List/Dict/HashSet, LINQ 기초 |
| D4_Iterators | yield return/break, 지연 평가 |
| D5_LINQ | Select/Where/GroupBy/Join/Aggregate |
| D6_Interfaces | IEnumerable/IEnumerator 직접 구현 |
| D7_AdvancedIteration | 무한 시퀀스, 배치 처리, 파이프라인 |

### 5. Containers
| 파일 | 내용 |
|------|------|
| D1_SpanMemory | Span\<T\>, Memory\<T\>, MemoryMarshal |
| D2_CustomContainers | 링크드 리스트, 스택, 큐 직접 구현 |
| D3_TreeGraph | 이진 트리, 그래프, BFS/DFS |
| D4_BuiltinAdvanced | ImmutableArray, FrozenDictionary, PriorityQueue |
| D5_Patterns | Rope, Trie, 슬라이딩 윈도우 |

### 6. ExceptionHandling
| 파일 | 내용 |
|------|------|
| D1_TryCatchFinally | 기본 예외 흐름, finally 보장 |
| D2_CustomExceptions | 커스텀 예외 계층 |
| D3_ErrorPatterns | `Result<T,TError>` (Map/Bind/Match), `Option<T>` |
| D4_ExceptionFilters | when 필터, 조건부 catch |
| D5_AdvancedExceptions | AggregateException, ExceptionDispatchInfo, ObjectDisposedException.ThrowIf |

```csharp
// 핵심 타입 (모두 public readonly struct)
public readonly struct Result<T, TError> { ... Map/Bind/Match ... }
public readonly struct Option<T> { ... Some/None/Map/GetValueOrDefault ... }
```

### 7. StringsAndIO
| 파일 | 내용 |
|------|------|
| D1_Strings | String vs StringBuilder, Span, 정규식 |
| D2_DateTime | DateOnly/TimeOnly, TimeZoneInfo, DateTimeOffset |
| D3_FileIO | File/Directory API, StreamReader/Writer, async |
| D4_Json | System.Text.Json — 직렬화 옵션, JsonIgnore, `JsonConverter<T>` |
| D5_Encoding | Encoding/Base64/Hex, `ArrayPool<byte>` 버퍼 재사용 |

### 8. Memory
| 파일 | 내용 |
|------|------|
| D1_GCBasics | GC 세대, 강제 수집, GC.GetTotalMemory |
| D2_LOH | Large Object Heap, 85KB 임계값 |
| D3_Finalizer | finalizer 패턴, GC.SuppressFinalize |
| D4_Dispose | IDisposable, using, IAsyncDisposable |
| D5_UnsafeMemory | unsafe 블록, fixed, MemoryMarshal, stackalloc |

### 9. Async
| 파일 | 내용 |
|------|------|
| D1_TaskFundamentals | Task.Run, Task.WhenAll/WhenAny, 연속 |
| D2_AsyncAwait | async/await 흐름, ConfigureAwait, SynchronizationContext |
| D3_ValueTask | `ValueTask<T>` vs `Task<T>`, `AsyncCache<TKey,TValue>` (zero-alloc fast path) |
| D4_Cancellation | CancellationToken, ThrowIfCancellationRequested, linked source |
| D5_AsyncStreams | `IAsyncEnumerable<T>`, `[EnumeratorCancellation]`, await foreach |

### 10. Concurrency
| 파일 | 내용 |
|------|------|
| D1_Thread | Thread, ThreadPool, BackgroundWorker |
| D2_Task | Parallel.For/ForEach, PLINQ |
| D3_Parallel | 데이터 병렬화, 파티셔닝 |
| D4_Sync | lock, Monitor, Mutex, SemaphoreSlim, Interlocked |
| D5_Collections | ConcurrentDictionary, ConcurrentQueue, BlockingCollection |
| D6_AdvancedChannels | `Channel<T>`, 생산자-소비자 파이프라인 |
| D7_AsyncStreams | 비동기 스트림과 동시성 결합 |
| D8_MemoryAndContext | ThreadLocal, AsyncLocal, ExecutionContext |

**특이사항**: `FSharpLib/` — F# 비동기 계산 식을 C# Parallel 코드와 통합하는 예제

### 11. HostBuilder
| 구성 요소 | 내용 |
|-----------|------|
| Services/ | IHostedService, BackgroundService, IGreetingService (DI 서비스 예제) |
| Options/ | AppSettings, IOptions\<T\> 패턴 |
| Program.cs | Generic Host 구성, 서비스 등록, 수명 관리 |

**패키지**: Microsoft.Extensions.Hosting 10.0.8

### 12. Logger
| 구성 요소 | 내용 |
|-----------|------|
| Services/OrderService | 구조적 로깅, 로그 레벨 |
| Services/PaymentService | 로그 스코프, LoggerMessage source gen |
| Benchmarks/ | 레벨별 로깅 성능, ListLoggerProvider |

**패키지**: Microsoft.Extensions.Hosting 10.0.8

### 13. Testing — OutputType: Library
| 파일 | 내용 |
|------|------|
| Domain/OrderService.cs | IOrderRepository, IEmailService, IInventoryService, OrderService, PricingEngine |
| Tests/D1_XUnitPatterns | [Fact]/[Theory]/[InlineData], IClassFixture\<T\> |
| Tests/D2_MockingPatterns | Moq — Setup/Returns/ReturnsAsync/Verify/Callback/SetupSequence |
| Tests/D3_TestPatterns | 플루언트 빌더, TestDouble, 경계값 테스트 |

**패키지**: xunit 2.9.3, Moq 4.20.72

### 14. HttpClient — 🔶 진행 중
| 파일 | 내용 | 상태 |
|------|------|------|
| Demos/MockHttpHandler.cs | `MockHttpHandler : HttpMessageHandler` (Enqueue/SentRequests) | ✅ |
| Demos/D1_HttpClientBasics.cs | GET/POST/응답 처리/헤더/Singleton 패턴 | ✅ |
| Demos/D2_IHttpClientFactory.cs | Named · Typed client · LoggingHandler/AuthHandler 파이프라인 | ✅ |
| Demos/D3_Resilience.cs | Polly 재시도 · Circuit Breaker · Timeout · PolicyWrap 패턴 | ✅ |
| Demos/D4_Authentication.cs | Bearer · API Key · Basic · OAuth2 토큰 자동 갱신 (DelegatingHandler) | ✅ |
| Demos/D5_HttpClientAdvanced.cs | 스트리밍 다운로드 · JSON Lines · Multipart · 병렬 요청 · 취소 패턴 | ✅ |
| Program.cs | 진입점 | ⬜ |
| Tests/ | HttpClient 테스트 | ⬜ |
| Benchmarks/ | 벤치마크 | ⬜ |

**패키지**: Microsoft.Extensions.Http 10.0.0, Microsoft.Extensions.DependencyInjection 10.0.0, Polly.Extensions.Http 3.0.0

### 18. FSharpFunctional
F# 함수형 프로그래밍 — DU(판별 유니온), 파이프라인 연산자, 재귀, 계산 식(CE) 패턴.  
`Tests/Tests.fsproj` — F# 전용 테스트 프로젝트.

### 19. Roslyn
| 구성 요소 | 내용 |
|-----------|------|
| Demos/D1_SyntaxTree | SyntaxTree 파싱, 노드/토큰 탐색 |
| Demos/D2_SemanticModel | SemanticModel, 심볼 조회, 타입 정보 |
| Demos/D3_Analysis | 코드 분석, 경고 패턴 감지 |
| Demos/D4_SyntaxFactory | SyntaxFactory 코드 생성 |
| Demos/D5_Workspace | MSBuildWorkspace, 프로젝트 로드 |
| Analyzer/ | `DiagnosticAnalyzer` 커스텀 분석기 |
| Generator/ | `ISourceGenerator` 소스 생성기 |
| Consumer/ | 생성기 소비자 프로젝트 |

**패키지**: Microsoft.CodeAnalysis.CSharp.Workspaces, Microsoft.CodeAnalysis.CSharp.Scripting

---

## 미시작 모듈 계획

### 15. AspNetCore
- Minimal API (`MapGet/MapPost/MapPut/MapDelete`, route parameters, typed results)
- Controller 기반 API (`[ApiController]`, 모델 바인딩, 유효성 검사)
- 미들웨어 파이프라인 (`Use/Run/Map`, 요청/응답 파이프라인)
- Filters (Action / Exception / Authorization 필터)
- Authentication & Authorization (JWT Bearer, Policy 기반)
- `WebApplicationFactory<T>` 통합 테스트

### 16. EFCore
- `DbContext`, `DbSet<T>`, 모델 구성 (Fluent API / 어노테이션)
- Migrations (`Add-Migration`, `Update-Database`, 롤백)
- CRUD (`Add/FindAsync/Remove`, `SaveChangesAsync`)
- 쿼리 (`Include/ThenInclude`, `AsNoTracking`, raw SQL `FromSqlRaw`)
- 트랜잭션, 낙관적 동시성 (`[ConcurrencyCheck]`)
- In-Memory / SQLite Provider 기반 단위 테스트

### 17. Architecture
- Layered Architecture (Domain / Application / Infrastructure / Presentation)
- CQRS (Command/Query 분리, 직접 구현 또는 MediatR)
- Repository 패턴 + Unit of Work
- Domain Events (이벤트 발행/처리)
- Clean Architecture (의존성 역전, UseCase 레이어)

### 20. NativeAOT
- `<PublishAot>true</PublishAot>` 설정, 트리밍 경고 해소
- 반사 불가 API 제약 처리 (`[RequiresUnreferencedCode]`)
- `JsonSerializerContext` (소스 생성기 기반 직렬화)
- P/Invoke 마샬링 제약 (`[LibraryImport]` vs `[DllImport]`)
- AOT vs JIT 시작 시간 / 메모리 벤치마크

### 21. Observability
- `ILogger` 구조적 로깅 심화 (`LoggerMessage` source gen)
- `System.Diagnostics.Metrics` (`Meter`, `Counter<T>`, `Histogram<T>`)
- `ActivitySource` / `Activity` — 분산 추적 (W3C TraceContext)
- OpenTelemetry SDK 연동 (Tracing + Metrics 내보내기)
- `IHealthCheck` / `HealthCheckService` — 헬스체크 엔드포인트

---

## 공통 규칙 & 주의사항

### 공통 규칙
| 항목 | 값 |
|------|----|
| 타겟 프레임워크 | `net10.0` 통일 |
| Nullable | `enable` 전역 적용 |
| ImplicitUsings | `enable` |
| 테스트 프레임워크 | xunit 2.9.3 + Microsoft.NET.Test.Sdk 17.14.1 |
| 벤치마크 | BenchmarkDotNet |
| 테스트 스타일 | 짧고 약식 ([Fact] 위주, Theory는 핵심 케이스만) |
| Program.cs | `<Compile Remove="Tests/**" />`, `<Compile Remove="Benchmarks/**" />` 포함 |

### C# 주의사항 (이전 작업에서 실제 발생한 오류)
- **최상위 타입 기본 접근자**: `internal` — Tests 프로젝트에서 쓰는 타입은 반드시 `public` 명시
- **static 로컬 함수**: 외부 스코프 변수 캡처 불가 → 파라미터로 전달
- **`HttpRequestMessage` 재사용 불가**: 재시도 시 새 인스턴스 생성 필요
- **`ValueTask<T>` 복수 await 금지**: 동기 완료 경로에서만 힙 절약, 두 번 이상 await 시 예외 발생

### 알려진 이슈
- `11. Roslyn/Consumer/` — 빈 디렉터리가 유령 폴더로 잔존  
  실제 내용은 `19. Roslyn/`으로 이전 완료. VS 종료 후 `11. Roslyn/` 폴더 수동 삭제 필요.

---

## 솔루션 파일

`PracticeDotnet.slnx` — Visual Studio XML 솔루션 (slnx 포맷)  
모든 21개 모듈 폴더 등록 완료. 미시작 모듈(15~17, 20~21)은 폴더 항목만 선언된 상태.
