using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;

// ───────────────────────────────────────────────────────
// I/O 바운드 — 비동기 동시성 전략 비교
//
// SequentialAwait  : await가 완료될 때까지 다음으로 안 넘어감
//                    총 소요 = TaskCount × DelayMs
//
// WhenAll          : 모든 Task를 동시에 시작, 가장 늦은 것을 기다림
//                    총 소요 ≈ DelayMs (이론값)
//
// Throttled        : SemaphoreSlim으로 동시 수를 MaxConcurrency로 제한
//                    총 소요 ≈ ceil(TaskCount / MaxConcurrency) × DelayMs
//                    실전: HTTP 요청 수 제한, 외부 API 레이트 리밋 준수
// ───────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class IoBoundBench
{
    [Params(20)]
    public int TaskCount;

    private const int DelayMs = 10;
    private const int MaxConcurrency = 5; // Throttled 버전의 동시 제한

    // 총 소요: TaskCount × DelayMs = 200ms
    [Benchmark(Baseline = true)]
    public async Task SequentialAwait()
    {
        for (int i = 0; i < TaskCount; i++)
            await Task.Delay(DelayMs);
    }

    // 총 소요: ~DelayMs = 10ms (20배 차이)
    [Benchmark]
    public Task WhenAll()
    {
        var tasks = Enumerable.Range(0, TaskCount)
            .Select(_ => Task.Delay(DelayMs));
        return Task.WhenAll(tasks);
    }

    // 총 소요: ceil(20/5) × 10ms = 40ms
    // WhenAll보다 느리지만 Sequential보다 훨씬 빠름
    // 실전: 외부 서비스 과부하 방지, 레이트 리밋 준수
    [Benchmark]
    public async Task Throttled()
    {
        var semaphore = new SemaphoreSlim(MaxConcurrency, MaxConcurrency);
        var tasks = Enumerable.Range(0, TaskCount).Select(async _ =>
        {
            await semaphore.WaitAsync();
            try { await Task.Delay(DelayMs); }
            finally { semaphore.Release(); }
        });
        await Task.WhenAll(tasks);
    }
}
