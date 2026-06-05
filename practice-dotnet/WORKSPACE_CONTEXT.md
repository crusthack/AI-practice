# Workspace Context — practice-dotnet

> 다음 agent가 컨텍스트 없이 이 워크스페이스를 이어받을 때 읽을 파일.  
> 최종 업데이트: 2026-06-05

---

## 1. 저장소 개요

**.NET 10 학습용 멀티 프로젝트 모노레포.**  
C#으로 시작해서 Generic Host, 로깅, 멀티스레딩, 벤치마크, F# 라이브러리까지 단계적으로 쌓아가는 구조.

```
practice-dotnet/
├── PracticeDotnet.slnx          ← Visual Studio / Rider 솔루션 파일
├── README.md
├── DOTNET_LEARNING_ROADMAP.md   ← 30단계 장기 로드맵
├── WORKSPACE_CONTEXT.md         ← (이 파일)
│
├── 1. CSharpBasics/             ← C# 기본기 콘솔 앱
├── 2. HostBuilder/              ← Generic Host + DI + BackgroundService
├── 3. Logger/                   ← ILogger, 구조적 로깅
└── 4. MultiThreading/           ← 동시성/병렬성 학습 (가장 많이 작업됨)
```

**런타임**: net10.0 / .NET 10.0.8  
**SDK**: .NET SDK 10.0.300  
**도구**: BenchmarkDotNet 0.15.8, ilspycmd 10.1.0, xUnit

---

## 2. 솔루션 구조 (PracticeDotnet.slnx)

| 폴더 | 프로젝트 파일 | 어셈블리명 |
|------|-------------|-----------|
| /1. CSharpBasics/ | 1. CSharpBasics/1. CSharpBasics.csproj | - |
| /1. CSharpBasics/ | 1. CSharpBasics/Tests/Tests.csproj | - |
| /1. CSharpBasics/ | 1. CSharpBasics/Benchmarks/CSharpBench.csproj | CSharpBench |
| /2. HostBuilder/ | 2. HostBuilder/2. HostBuilder.csproj | - |
| /2. HostBuilder/ | 2. HostBuilder/Tests/Tests.csproj | - |
| /2. HostBuilder/ | 2. HostBuilder/Benchmarks/HostBench.csproj | HostBench |
| /3. Logger/ | 3. Logger/3. Logger.csproj | - |
| /3. Logger/ | 3. Logger/Tests/Tests.csproj | - |
| /3. Logger/ | 3. Logger/Benchmarks/LogBench.csproj | LogBench |
| /4. MultiThreading/ | 4. MultiThreading/4. MultiThreading.csproj | - |
| /4. MultiThreading/ | 4. MultiThreading/Tests/Tests.csproj | - |
| /4. MultiThreading/ | 4. MultiThreading/Benchmarks/MTBench.csproj | MTBench |
| /4. MultiThreading/ | 4. MultiThreading/FSharpLib/FSharpLib.fsproj | FSharpLib |

> 주의: 각 Benchmark 프로젝트는 서로 다른 AssemblyName을 가진다. 이전에 모두 "Benchmarks"였다가 BenchmarkDotNet 충돌로 고유명으로 변경됨.

---

## 3. 각 프로젝트 상세

### 3.1 1. CSharpBasics

- **목적**: C# 기본 문법 실습
- **주요 파일**: `TextUtils.cs`, `NumberUtils.cs`, `Program.cs`
- **테스트**: `Tests/TextUtilsTests.cs`, `Tests/NumberUtilsTests.cs`
- **벤치마크**: `Benchmarks/LinqBench.cs`, `Benchmarks/StringOpsBench.cs`
- **실행**: `dotnet run --project "1. CSharpBasics"`

### 3.2 2. HostBuilder

- **목적**: Generic Host, DI, BackgroundService 학습
- **주요 파일**: `Services/GreetingService.cs`, `Services/PeriodicWorker.cs`, `Options/AppSettings.cs`
- **테스트**: `Tests/GreetingServiceTests.cs`, `Tests/ServiceLifetimeTests.cs`
- **벤치마크**: `Benchmarks/DiResolutionBench.cs`

### 3.3 3. Logger

- **목적**: ILogger, 구조적 로깅
- **주요 파일**: `Services/OrderService.cs`, `Services/PaymentService.cs`
- **테스트**: `Tests/OrderServiceTests.cs`
- **벤치마크**: `Benchmarks/LoggingBench.cs`

### 3.4 4. MultiThreading — 가장 많이 작업된 프로젝트

이전 세션들에서 핵심 작업이 여기서 이루어졌다. 아래 4.x 섹션에서 상세 설명.

---

## 4. 4. MultiThreading 상세

### 4.1 Demos (데모 코드)

```
4. MultiThreading/Demos/
├── D1_Thread.cs    ← Thread 기본
├── D2_Task.cs      ← Task / async-await
├── D3_Parallel.cs  ← Parallel.For / ForEach
├── D4_Sync.cs      ← lock, SemaphoreSlim
├── D5_Collections.cs ← ConcurrentDictionary, BlockingCollection
└── Print.cs        ← 출력 헬퍼
```

### 4.2 Benchmarks (MTBench.csproj)

```
4. MultiThreading/Benchmarks/
├── Program.cs            ← BenchmarkSwitcher (--filter 지원)
├── B1_ThreadVsTask.cs    → ThreadVsTaskBench
├── B2_CpuBound.cs        → CpuBoundBench
├── B3_IoBound.cs         → IoBoundBench
├── B4_SharedState.cs     → SharedStateBench
├── B5_ReadHeavy.cs       → ReadHeavyBench
├── B6_ProducerConsumer.cs → ProducerConsumerBench
└── B7_FSharpVsCSharp.cs  → ParallelAggregateBench, ActorCounterBench,
                             FSharpOptParallelBench, FSharpOptActorBench
```

**실행법**:
```powershell
# 전체 실행 (시간 매우 오래 걸림)
dotnet run -c Release --project "4. MultiThreading/Benchmarks"

# 특정 벤치마크만
dotnet run -c Release --project "4. MultiThreading/Benchmarks" -- --filter "*FSharpOpt*" --join
dotnet run -c Release --project "4. MultiThreading/Benchmarks" -- --filter "*ParallelAggregate*"
```

### 4.3 FSharpLib (FSharpLib.fsproj)

F# 라이브러리 — C# 벤치마크에서 참조됨.

```
4. MultiThreading/FSharpLib/
├── FsParallel.fs     ← 순진한 Array.Parallel.map 패턴 (느림, 비교 기준)
├── FsActor.fs        ← MailboxProcessor 액터 (async {} CE, 느림)
├── FsAsync.fs        ← throttledParallel, retryAsync 유틸
├── FsParallelOpt.fs  ← 최적화 병렬 패턴 (inlineThreadLocal, spanPartitioned, taskParallel, plinq)
└── FsTaskActor.fs    ← task {} + Channel<int> 액터 (TaskChannelCounter, ConcurrentTaskCounter)
```

**F# 컴파일 순서**: fsproj의 `<Compile Include>` 순서가 중요. 위 순서대로 컴파일됨.

### 4.4 벤치마크 결과 (실측, 2026-06-05)

**환경**: AMD Ryzen 7 8845HS (8C/16T) · .NET 10.0.8 · ShortRun(3 iter)

#### FSharpOptParallelBench — 병렬 제곱합

| Method | Size | Mean | C# ThreadLocal 대비 | Allocated |
|--------|-----:|-----:|-------------------:|----------:|
| **C# ThreadLocal** *(baseline)* | 500K | 236 µs | 1.00× | 5 KB |
| F# InlineThreadLocal | 500K | 229 µs | 1.03× ✓ | 5 KB |
| **F# SpanPartitioned** | 500K | **89 µs** | **2.65×** ↑↑ | 5 KB |
| **F# TaskParallel** | 500K | **99 µs** | **2.38×** ↑↑ | 3 KB |
| F# PLINQ | 500K | 399 µs | 0.59× | 9 KB |
| F# Original (Array.Parallel.map) | 500K | 891 µs | 0.26× ✗ | **3,933 KB** |
| **C# ThreadLocal** *(baseline)* | 2M | 799 µs | 1.00× | 5 KB |
| F# InlineThreadLocal | 2M | 739 µs | 1.08× ✓ | 5 KB |
| **F# SpanPartitioned** | 2M | **332 µs** | **2.41×** ↑↑ | 5 KB |
| **F# TaskParallel** | 2M | **262 µs** | **3.05×** ↑↑ | 3 KB |
| F# PLINQ | 2M | 1,327 µs | 0.60× | 9 KB |
| F# Original (Array.Parallel.map) | 2M | 2,248 µs | 0.36× ✗ | **15,650 KB** |

#### FSharpOptActorBench — 액터 카운터 (50K 메시지)

| Method | Mean | C# Channel 대비 | Allocated |
|--------|-----:|----------------:|----------:|
| **C# Channel** *(baseline)* | 2,539 µs | 1.00× | 11 KB |
| F# MailboxProcessor | 13,062 µs | 0.19× ✗ | **27,245 KB** |
| **F# TaskChannel** | **1,191 µs** | **2.13×** ↑↑ | 261 KB |
| F# ConcurrentTaskChannel | 2,339 µs | 1.09× ✓ | 36 KB |

**핵심 발견**: 올바른 F# 패턴 (SpanPartitioned, TaskParallel)은 C# 최적 패턴보다 2~3× 빠름.

### 4.5 분석 보고서

`4. MultiThreading/FSharp_IL_Report.md` — IL 수준 원인 분석 + 최적화 결과 포함  
원본 벤치마크 결과: `4. MultiThreading/BenchmarkDotNet.Artifacts/results/`

---

## 5. 주요 기술 컨텍스트

### F# async {} vs task {} CE

```
F# async {} → CPS (Continuation-Passing Style)
  - let! 마다 continuation 클로저 힙 할당
  - AsyncPrimitives.Bind 체인
  - 50K 메시지 → 27,245 KB 할당

F# task {} → IAsyncStateMachine (C# async/await와 동일 IL)
  - 메서드당 상태 기계 1개 (힙 객체 1개)
  - ValueTask.IsCompleted=true → 동기 계속 (힙 할당 없음)
  - 50K 메시지 → 261 KB 할당
```

### F# byref 제약

F#에서 `let mutable` 캡처 변수의 주소(`&x`)를 클로저 안에서 취할 수 없다.  
`Parallel.For` finalizer에서 `Interlocked.Add(&acc, local)` 호출이 필요할 때 해결책:

```fsharp
[<Sealed>]
type private Acc() =
    [<DefaultValue>] val mutable Val: int64
// &acc.Val 은 힙 필드 주소 → 클로저 안에서 사용 가능
```

### Array.Parallel.map 할당 원인

`Array.Parallel.map f data` → `ArrayModule.Parallel.Map<'T,'U>` → 결과 `'U[]` 즉시 힙 할당.  
500K 원소 int64 배열 = 4 MB 중간 배열. 이를 피하려면 `Parallel.For(0, P, ...)` 직접 사용.

### BenchmarkDotNet --filter 사용법

`Program.cs`에서 `args.Length > 0` 분기로 `BenchmarkSwitcher.FromAssembly().Run(args)` 호출.  
`BenchmarkRunner.Run<T>()` 개별 호출 시 `--filter`가 무시됨.

---

## 6. 미완료 항목 / 다음 작업 후보

### 즉시 가능한 작업
- [ ] `3. Logger` 프로젝트 실제 구조적 로깅 예제로 구현 (현재 Hello World 수준)
- [ ] F# `[<Struct>]` DU 적용 — FsActor.fs의 CounterMsg를 struct DU로 변환해서 힙 할당 감소 측정
- [ ] `FsAsync.fs`의 `throttledParallel`/`retryAsync` 벤치마크 추가

### 중기 작업 (DOTNET_LEARNING_ROADMAP.md 참고)
- [ ] `5. WebApi` 프로젝트 — ASP.NET Core Minimal API + EF Core SQLite
- [ ] `6. AsyncConcurrency` — Channel 기반 background queue 심화
- [ ] `functional/` 프로젝트 정리 (현재 별도 경로에 있음, FSharpLib과 통합 고려)

### 보고서 확장 가능
- `FSharp_IL_Report.md`에 ilspycmd 실제 IL 덤프 추가 (현재는 설명 중심)
- `[<Struct>]` DU 적용 후 결과 비교 섹션 추가

---

## 7. 주의사항

1. **Benchmark 실행은 반드시 Release 모드**: `dotnet run -c Release --project ...`
2. **Benchmark 프로젝트 AssemblyName 고유화**: 4개 모두 다른 이름 (CSharpBench, HostBench, LogBench, MTBench)
3. **F# 파일 컴파일 순서**: `.fsproj`의 `<Compile Include>` 순서가 의존성 순서 — 재배치 시 오류 발생 가능
4. **ilspycmd**: 전역 설치 필요. `dotnet tool install -g ilspycmd`
5. **백그라운드 벤치마크 프로세스**: 벤치마크 중단 시 DLL lock 발생. `Get-Process -Name "MTBench" | Stop-Process -Force`로 해제

---

## 8. 빠른 시작 명령어

```powershell
# 솔루션 빌드 확인
dotnet build

# 4. MultiThreading 테스트 실행
dotnet test "4. MultiThreading/Tests/Tests.csproj"

# F# + C# 최적화 벤치마크만 실행 (~2분)
dotnet run -c Release --project "4. MultiThreading/Benchmarks" -- --filter "*FSharpOpt*" --join

# 전체 MultiThreading 벤치마크 (~20분)
dotnet run -c Release --project "4. MultiThreading/Benchmarks"

# IL 분석 (FSharpLib.dll)
ilspycmd "4. MultiThreading/FSharpLib/bin/Release/net10.0/FSharpLib.dll" -p
```
