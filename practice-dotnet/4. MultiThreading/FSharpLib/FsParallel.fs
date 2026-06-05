/// 불변(immutable) 데이터 기반 순수 병렬 연산
/// 공유 상태가 없으므로 lock / Interlocked 불필요
module FSharpLib.FsParallel

open System

// ── 병렬 제곱합
// Array.Parallel.map: 내부적으로 Parallel.For 사용하나
// 각 워커가 새 배열 원소에 쓸 뿐 — 공유 가변 상태 없음.
let sumSquares (data: int[]) : int64 =
    data
    |> Array.Parallel.map (fun x -> int64 x * int64 x)
    |> Array.sum                                        // sum도 순차이나 중간값은 이미 불변 배열

// ── 병렬 조건 카운트 (map → sum 패턴)
// C# 에서 Interlocked.Increment 가 필요한 경우를 map+sum 으로 대체
let countIf (predicate: Func<int, bool>) (data: int[]) : int =
    data
    |> Array.Parallel.map (fun x -> if predicate.Invoke(x) then 1 else 0)
    |> Array.sum

// ── 병렬 파티션 집계 (thread-local reduce 패턴)
// Parallel.For의 localInit/localFinally 패턴을 단순화한 FP 등가
let partitionedSum (chunkSize: int) (data: int[]) : int64 =
    data
    |> Array.chunkBySize chunkSize                     // 청크 분할 (불변 배열 슬라이스)
    |> Array.Parallel.map (fun chunk ->
        chunk |> Array.sumBy (fun x -> int64 x * int64 x))
    |> Array.sum
