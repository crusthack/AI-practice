using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using System.Collections.Concurrent;
using System.Threading.Channels;

// ───────────────────────────────────────────────────────────────────────────
// F# 함수형 vs C# 명령형  ─  병렬 패턴 비교
//
// [비교 1] 병렬 집계 (SumSquares)
//   C# 명령형  : Parallel.For + Interlocked.Add  (공유 변수 + 원자 연산)
//   C# thread-local: Parallel.For localInit/Finally (스레드별 누적 후 합산)
//   F# 순수형  : Array.Parallel.map → Array.sum   (불변 중간 배열, lock 없음)
//   F# 청크형  : Array.chunkBySize → Parallel.map → sum
//
// [비교 2] 액터 카운터 (ActorCounter)
//   C# Channel  : Channel<int> + Task 소비자     (lock-free 메시지 큐)
//   F# Mailbox  : MailboxProcessor (actor loop)  (락 없는 상태 캡슐화)
//
// 핵심 관찰
//   • F# Array.Parallel.map은 Parallel.For와 동일한 IL을 생성하지만
//     소스 레벨에서 공유 가변 상태를 쓸 수 없게 강제한다.
//   • MailboxProcessor는 Channel<T> + dedicated consumer와 동일한 스루풋이나
//     외부 코드가 내부 상태를 직접 변경할 수 없다는 타입 보장이 있다.
// ───────────────────────────────────────────────────────────────────────────

[MemoryDiagnoser]
[ShortRunJob]
public class ParallelAggregateBench
{
    [Params(500_000, 2_000_000)]
    public int Size;

    private int[] _data = null!;

    [GlobalSetup]
    public void Setup() => _data = Enumerable.Range(1, Size).ToArray();

    // ── 기준선: Parallel.For + 전역 원자 연산
    [Benchmark(Baseline = true)]
    public long CSharp_ParallelFor_Interlocked()
    {
        long total = 0;
        Parallel.For(0, _data.Length, i =>
            Interlocked.Add(ref total, (long)_data[i] * _data[i]));
        return total;
    }

    // ── C# thread-local 누적 → 최종 합산 (권장 C# 패턴)
    [Benchmark]
    public long CSharp_ParallelFor_ThreadLocal()
    {
        long total = 0;
        Parallel.For(
            0, _data.Length,
            () => 0L,
            (i, _, local) => local + (long)_data[i] * _data[i],
            local => Interlocked.Add(ref total, local));
        return total;
    }

    // ── C# PLINQ (선언형, thread-local 내부 사용)
    [Benchmark]
    public long CSharp_PLINQ()
        => _data.AsParallel().Select(x => (long)x * x).Sum();

    // ── F# 순수 병렬: Array.Parallel.map → Array.sum
    //    중간 배열에 쓰기만 하므로 lock 불필요
    [Benchmark]
    public long FSharp_ImmutableParallel()
        => FSharpLib.FsParallel.sumSquares(_data);

    // ── F# 청크 파티션: chunkBySize → Parallel.map → sum
    //    thread-local 패턴을 FP 스타일로 표현
    [Benchmark]
    public long FSharp_ChunkedParallel()
        => FSharpLib.FsParallel.partitionedSum(Environment.ProcessorCount * 8, _data);
}

[MemoryDiagnoser]
[ShortRunJob]
public class ActorCounterBench
{
    [Params(50_000)]
    public int Messages;

    // ── C# Channel<T> 기반 카운터
    [Benchmark(Baseline = true)]
    public async Task<int> CSharp_Channel()
    {
        var channel = Channel.CreateUnbounded<int>(
            new UnboundedChannelOptions { SingleReader = true });
        int count = 0;

        // 소비자 태스크
        var consumer = Task.Run(async () =>
        {
            await foreach (var delta in channel.Reader.ReadAllAsync())
                count += delta;
        });

        // 생산자
        for (int i = 0; i < Messages; i++)
            await channel.Writer.WriteAsync(1);
        channel.Writer.Complete();

        await consumer;
        return count;
    }

    // ── F# MailboxProcessor 기반 카운터
    [Benchmark]
    public async Task<int> FSharp_MailboxProcessor()
    {
        using var agent = new FSharpLib.FsActor.CounterAgent();
        for (int i = 0; i < Messages; i++)
            agent.Increment(1);
        return await agent.GetCountAsync();
    }

    // ── C# ConcurrentDictionary → 참고용 (완전히 다른 용도지만 락 없는 기준)
    [Benchmark]
    public int CSharp_Interlocked_Baseline()
    {
        int count = 0;
        Parallel.For(0, Messages, _ => Interlocked.Increment(ref count));
        return count;
    }
}

// ───────────────────────────────────────────────────────────────────────────
// [최적화 비교] F# 병렬 집계 — 중간 배열 없는 패턴들
//
// 기준: C# ThreadLocal (할당 최소, 가장 빠른 C# 패턴)
// 비교:
//   FSharp_InlineThreadLocal : Parallel.For + thread-local (F# 직접 표현)
//   FSharp_SpanPartitioned   : CPU당 1청크 수동 파티셔닝 (캐시 친화)
//   FSharp_TaskParallel      : Task.Run P개 + WhenAll (task {} 상태 기계)
//   FSharp_PLINQ             : data.AsParallel().Select().Sum() (C# PLINQ 등가)
// ───────────────────────────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class FSharpOptParallelBench
{
    [Params(500_000, 2_000_000)]
    public int Size;

    private int[] _data = null!;

    [GlobalSetup]
    public void Setup() => _data = Enumerable.Range(1, Size).ToArray();

    // 기준선: C# thread-local (최적 C# 패턴)
    [Benchmark(Baseline = true)]
    public long CSharp_ThreadLocal()
    {
        long total = 0;
        Parallel.For(0, _data.Length,
            () => 0L,
            (i, _, local) => local + (long)_data[i] * _data[i],
            local => Interlocked.Add(ref total, local));
        return total;
    }

    // F# Parallel.For + thread-local (중간 배열 없음)
    [Benchmark]
    public long FSharp_InlineThreadLocal()
        => FSharpLib.FsParallelOpt.inlineThreadLocalSum(_data);

    // F# 수동 CPU-청크 파티셔닝 (캐시 친화)
    [Benchmark]
    public long FSharp_SpanPartitioned()
        => FSharpLib.FsParallelOpt.spanPartitionedSum(_data);

    // F# Task.Run P개 병렬 (task {} 상태 기계)
    [Benchmark]
    public long FSharp_TaskParallel()
        => FSharpLib.FsParallelOpt.taskParallelSum(_data);

    // F# PLINQ (C# AsParallel() 동일)
    [Benchmark]
    public long FSharp_PLINQ()
        => FSharpLib.FsParallelOpt.plinqSum(_data);

    // 이전 F# (중간 배열 있음) — 개선 폭 확인용
    [Benchmark]
    public long FSharp_Original_Immutable()
        => FSharpLib.FsParallel.sumSquares(_data);
}

// ───────────────────────────────────────────────────────────────────────────
// [최적화 비교] F# 액터 카운터 — task {} CE 기반
//
// FSharp_MailboxProcessor (기존): async CPS → 메시지당 클로저 힙 할당
// FSharp_TaskChannel (최적화):    task {} 상태 기계 + Channel<int> 값타입 큐
// FSharp_ConcurrentChannel:       배경 소비자 task 시작 (C# 패턴과 동일 구조)
// ───────────────────────────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class FSharpOptActorBench
{
    [Params(50_000)]
    public int Messages;

    // 기준선: C# Channel<T> (동시 생산자/소비자)
    [Benchmark(Baseline = true)]
    public async Task<int> CSharp_Channel()
    {
        var ch = Channel.CreateUnbounded<int>(new UnboundedChannelOptions { SingleReader = true });
        int count = 0;
        var consumer = Task.Run(async () =>
        {
            await foreach (var n in ch.Reader.ReadAllAsync())
                count += n;
        });
        for (int i = 0; i < Messages; i++)
            ch.Writer.TryWrite(1);
        ch.Writer.Complete();
        await consumer;
        return count;
    }

    // 이전 F# MailboxProcessor (비교 기준 — 느림)
    [Benchmark]
    public async Task<int> FSharp_MailboxProcessor()
    {
        using var agent = new FSharpLib.FsActor.CounterAgent();
        for (int i = 0; i < Messages; i++)
            agent.Increment(1);
        return await agent.GetCountAsync();
    }

    // 최적화: task {} + Channel<int> (drain-after-produce)
    [Benchmark]
    public async Task<int> FSharp_TaskChannel()
    {
        using var counter = new FSharpLib.FsTaskActor.TaskChannelCounter();
        for (int i = 0; i < Messages; i++)
            counter.Increment(1);
        return await counter.GetCountAsync();
    }

    // 최적화: task {} + 배경 소비자 (동시 생산/소비 — C# 패턴과 동일)
    [Benchmark]
    public async Task<int> FSharp_ConcurrentTaskChannel()
    {
        using var counter = new FSharpLib.FsTaskActor.ConcurrentTaskCounter();
        for (int i = 0; i < Messages; i++)
            counter.Increment(1);
        return await counter.GetCountAsync();
    }
}
