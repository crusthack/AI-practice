/// MailboxProcessor — F# 액터 모델
/// 메시지 패싱으로 공유 상태를 캡슐화.
/// C# 의 Channel<T> + 전용 소비자 루프와 동일한 패턴이나
/// 타입 시스템으로 외부 변이를 원천 차단.
module FSharpLib.FsActor

open System.Threading
open System.Threading.Tasks

// 내부 메시지 타입 — discriminated union (C# enum 대비 타입 안전)
type private CounterMsg =
    | Increment of delta: int
    | GetCount  of reply: AsyncReplyChannel<int>
    | Reset

/// 잠금 없는 카운터 — MailboxProcessor 기반
/// 모든 상태 변경은 단일 루프에서 순차 처리 → race condition 불가
type CounterAgent() =
    let agent =
        MailboxProcessor<CounterMsg>.Start(fun inbox ->
            let rec loop n = async {
                let! msg = inbox.Receive()
                match msg with
                | Increment delta  -> return! loop (n + delta)
                | GetCount  reply  -> reply.Reply(n); return! loop n
                | Reset            -> return! loop 0
            }
            loop 0)

    member _.Increment(delta: int) = agent.Post(Increment delta)
    member _.Reset()               = agent.Post(Reset)

    /// 동기 카운트 조회 (PostAndReply — 블로킹)
    member _.GetCount() = agent.PostAndReply(GetCount)

    /// 비동기 카운트 조회 — C# 에서 await 가능
    member _.GetCountAsync() : Task<int> =
        agent.PostAndAsyncReply(GetCount) |> Async.StartAsTask

    interface System.IDisposable with
        member _.Dispose() = (agent :> System.IDisposable).Dispose()

/// N개 CounterAgent를 병렬로 실행하고 전체 합산
let runAgentBatch (agentCount: int) (incrementsPerAgent: int) : int =
    let agents = Array.init agentCount (fun _ -> new CounterAgent())

    // 각 에이전트에 incrementsPerAgent 번 메시지 발송
    agents |> Array.Parallel.iter (fun agent ->
        for _ in 1 .. incrementsPerAgent do
            agent.Increment(1))

    // 모든 에이전트의 카운트를 비동기로 수집
    let total =
        agents
        |> Array.map (fun a -> a.GetCountAsync() |> Async.AwaitTask)
        |> Async.Parallel
        |> Async.RunSynchronously
        |> Array.sum

    agents |> Array.iter (fun a -> (a :> System.IDisposable).Dispose())
    total
