/// 최적화된 F# 병렬 집계
/// 핵심: 중간 배열(intermediate array) 없이 thread-local 패턴을 F#으로 직접 구현
module FSharpLib.FsParallelOpt

open System
open System.Linq
open System.Threading
open System.Threading.Tasks

// thread-local 합산의 최종 Interlocked.Add 에 사용할 힙 축적기
// closure 안에서 &acc.Val 로 byref<int64> 취득 가능 (힙 필드 주소)
[<Sealed>]
type private Acc() =
    [<DefaultValue>] val mutable Val: int64

// ── 최적화 1: Parallel.For + thread-local (중간 배열 없음)
// C# ThreadLocal 패턴과 완전히 동일한 IL 전략 — F# 방식으로 표현
// 할당: Acc(~24 B) + 3개 Func/Action 클로저 (~200 B 합계)
// vs FSharp_ImmutableParallel: N × 8 bytes 중간 배열 없음
let inlineThreadLocalSum (data: int[]) : int64 =
    let acc = Acc()
    Parallel.For(
        0, data.Length,
        Func<int64>(fun () -> 0L),
        Func<int, ParallelLoopState, int64, int64>(fun i _ local ->
            local + int64 data.[i] * int64 data.[i]),
        Action<int64>(fun local ->
            Interlocked.Add(&acc.Val, local) |> ignore)
    ) |> ignore
    acc.Val

// ── 최적화 2: CPU당 1청크 수동 파티셔닝 (캐시 친화 + 오버헤드 최소)
// Parallel.For(0, ProcessorCount, ...) — P개 작업만 생성
// 각 파티션이 연속 메모리 구간을 순회 → L1/L2 캐시 히트율 극대화
// Parallel.For auto-partitioner 대비: 루프 오버헤드와 스케줄러 호출 감소
let spanPartitionedSum (data: int[]) : int64 =
    let acc = Acc()
    let n = data.Length
    let p = Environment.ProcessorCount
    let chunkSize = max 1 ((n + p - 1) / p)

    Parallel.For(
        0, p,
        Func<int64>(fun () -> 0L),
        Func<int, ParallelLoopState, int64, int64>(fun tid _ _ ->
            // 각 파티션의 시작/끝 인덱스 계산 (연속 구간)
            let s = tid * chunkSize
            let e = min n (s + chunkSize)
            let mutable local = 0L
            let mutable i = s
            while i < e do
                let v = int64 data.[i]
                local <- local + v * v
                i <- i + 1
            local),
        Action<int64>(fun local ->
            Interlocked.Add(&acc.Val, local) |> ignore)
    ) |> ignore
    acc.Val

// ── 최적화 3: Task.Run 명시적 병렬 — async CE CPS 오버헤드 없음
// Task.Run의 람다는 순수 동기 루프 → 상태 기계 전환 없음
// task {} CE 와 동일한 효과 (실제로 task {} 를 사용하면 내부에 await 없어 무의미)
// 할당: P개 Task 객체 (~3 KB) — 중간 배열 없음
let taskParallelSum (data: int[]) : int64 =
    let n = data.Length
    let p = Environment.ProcessorCount
    let chunkSize = max 1 ((n + p - 1) / p)

    Array.init p (fun tid ->
        let s = tid * chunkSize
        let e = min n (s + chunkSize)
        Task.Run(fun () ->
            let mutable acc = 0L
            let mutable i = s
            while i < e do
                let v = int64 data.[i]
                acc <- acc + v * v
                i <- i + 1
            acc))
    |> Task.WhenAll
    |> fun t -> t.Result
    |> Array.sum

// ── 최적화 4: PLINQ 등가 — F# 파이프라인에서 AsParallel 활용
// 중간 배열 없이 PLINQ 내부 파티셔너 사용
// C# PLINQ 와 완전히 동일한 IL — F# 스타일로 표현만 다름
let plinqSum (data: int[]) : int64 =
    data.AsParallel()
        .Select(fun x -> int64 x * int64 x)
        .Sum()
