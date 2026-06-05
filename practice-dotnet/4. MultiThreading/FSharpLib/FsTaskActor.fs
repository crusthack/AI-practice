/// task {} CE 기반 액터 — MailboxProcessor 대안
///
/// MailboxProcessor + async {} 의 문제:
///   • async {} → CPS 체인: let! 마다 continuation 클로저 힙 할당
///   • DU 메시지 (Increment of int) → sealed class 인스턴스 힙 할당
///   • 50K 메시지 = 27,245 KB 할당
///
/// 이 모듈의 접근:
///   • Channel<int> — int 값타입 큐 (박싱 없음)
///   • task {} → C# IAsyncStateMachine (MoveNext 상태 기계, CPS 없음)
///   • WaitToReadAsync 의 ValueTask 경로: 항목이 있으면 동기 완료 (yield 없음)
module FSharpLib.FsTaskActor

open System.Threading.Channels
open System.Threading.Tasks

// ── task {} 기반 Channel 카운터
// IDisposable 구현 — using 블록으로 사용 가능
type TaskChannelCounter() =
    let ch =
        Channel.CreateUnbounded<int>(
            UnboundedChannelOptions(SingleReader = true))

    /// 메시지 발송 — TryWrite는 동기, 잠금 없음
    member _.Increment(delta: int) =
        ch.Writer.TryWrite(delta) |> ignore

    /// 모든 Increment 완료 후 호출.
    /// Writer를 닫고 버퍼를 소진한 뒤 합계를 반환.
    ///
    /// 핵심 IL 특성:
    ///   task {} → IAsyncStateMachine.MoveNext() — 상태 int 필드 하나로 resume 지점 관리
    ///   let! (ValueTask<bool>) → IsCompleted = true면 동기 계속 (실제 yield 없음)
    ///   상태 기계는 한 번 힙 할당 후 재사용 (MailboxProcessor 처럼 메시지마다 클로저 없음)
    member _.GetCountAsync() : Task<int> = task {
        ch.Writer.Complete()
        let mutable total = 0
        let mutable running = true
        // ReadAllAsync().GetAsyncEnumerator() 를 통해 드레인
        // 버퍼가 채워진 상태라면 MoveNextAsync() 는 동기 완료 경로로 실행
        let e = ch.Reader.ReadAllAsync().GetAsyncEnumerator()
        while running do
            let! hasNext = e.MoveNextAsync()   // ValueTask<bool>: 버퍼에 항목 있으면 IsCompleted=true
            if hasNext then total <- total + e.Current
            else running <- false
        // Note: DisposeAsync 생략 — channel 수명 짧고, 이미 Complete 됨
        return total
    }

    interface System.IDisposable with
        member _.Dispose() = ch.Writer.TryComplete() |> ignore


// ── 동시 생산자/소비자 패턴 (C# CSharp_Channel 과 동일 구조)
// 소비자 task가 배경에서 시작하고, Complete() 시 종료
type ConcurrentTaskCounter() =
    let ch =
        Channel.CreateUnbounded<int>(
            UnboundedChannelOptions(SingleReader = true))
    let mutable _total = 0

    // 소비자: 객체 생성 즉시 배경 시작 (task {} 는 hot start)
    let consumerTask : Task<unit> = task {
        let e = ch.Reader.ReadAllAsync().GetAsyncEnumerator()
        let mutable running = true
        while running do
            let! hasNext = e.MoveNextAsync()
            if hasNext then _total <- _total + e.Current
            else running <- false
    }

    member _.Increment(delta: int) = ch.Writer.TryWrite(delta) |> ignore

    member _.GetCountAsync() : Task<int> = task {
        ch.Writer.Complete()
        do! consumerTask      // 소비자 완료 대기
        return _total
    }

    interface System.IDisposable with
        member _.Dispose() = ch.Writer.TryComplete() |> ignore
