/// F# async workflow 기반 비동기 유틸리티
/// C# Task/async-await 와 동일한 상태 기계로 컴파일되나
/// 합성(composition)이 더 자연스럽다.
module FSharpLib.FsAsync

open System
open System.Threading
open System.Threading.Tasks

// ── 최대 동시성 제한 병렬 실행 (F# async)
// C# 의 SemaphoreSlim + Task.WhenAll 패턴과 동일 역할
// 차이: 각 async {} 블록이 지연 계산(lazy) — 명시적 시작 전 실행 안 됨
let throttledParallel (maxConcurrency: int) (factories: (unit -> Async<'T>) list) : Task<'T[]> =
    let sem = new SemaphoreSlim(maxConcurrency)
    factories
    |> List.map (fun factory -> async {
        do! Async.AwaitTask(sem.WaitAsync())
        try  return! factory()
        finally sem.Release() |> ignore
    })
    |> Async.Parallel
    |> Async.StartAsTask

// ── 지수 백오프 재시도 (F# async 합성)
let retryAsync (maxAttempts: int) (initialDelayMs: int) (operation: unit -> Async<'T>) : Async<'T> =
    let rec go attempt = async {
        try
            return! operation()
        with _ when attempt < maxAttempts ->
            let delayMs = initialDelayMs * (pown 2 (attempt - 1))
            do! Async.Sleep delayMs
            return! go (attempt + 1)
    }
    go 1

// ── C# Task<T[]> 로 노출 (벤치마크에서 직접 await)
let throttledParallelTask (maxConcurrency: int) (delayMs: int) (count: int) : Task<int[]> =
    let factories =
        List.init count (fun i ->
            fun () -> async {
                do! Async.Sleep delayMs
                return i
            })
    throttledParallel maxConcurrency factories
