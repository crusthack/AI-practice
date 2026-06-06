using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;

// ───────────────────────────────────────────────────────
// Thread vs ThreadPool vs Task.Run — 작업 디스패치 오버헤드
//
// Thread               : OS 스레드 직접 생성 (스택 1MB, 커널 호출)
// ThreadPool.Queue...  : 풀에서 꺼내 쓰므로 생성 비용 없음
// Task.Run             : ThreadPool + 스케줄러 추상화 + 상태 머신
//
// 예측: ThreadPool ≈ Task.Run << Thread
// ───────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class ThreadVsTaskBench
{
    [Params(50)]
    public int Count;

    // 스레드당 수행하는 최소 CPU 작업 (생성 비용이 주된 측정 대상)
    private const int SpinCount = 500;

    // 방법 1: OS 스레드를 직접 생성하고 Join으로 대기
    [Benchmark(Baseline = true)]
    public void RawThread()
    {
        var threads = new Thread[Count];
        for (int i = 0; i < Count; i++)
        {
            threads[i] = new Thread(() => Thread.SpinWait(SpinCount))
            {
                IsBackground = true
            };
            threads[i].Start();
        }
        for (int i = 0; i < Count; i++)
            threads[i].Join();
    }

    // 방법 2: ThreadPool에 직접 큐잉
    [Benchmark]
    public void ThreadPoolQueue()
    {
        using var countdown = new CountdownEvent(Count);
        for (int i = 0; i < Count; i++)
        {
            ThreadPool.QueueUserWorkItem(_ =>
            {
                Thread.SpinWait(SpinCount);
                countdown.Signal();
            });
        }
        countdown.Wait();
    }

    // 방법 3: Task.Run (ThreadPool 위의 추상화)
    [Benchmark]
    public async Task TaskRun()
    {
        var tasks = new Task[Count];
        for (int i = 0; i < Count; i++)
            tasks[i] = Task.Run(() => Thread.SpinWait(SpinCount));
        await Task.WhenAll(tasks);
    }
}
