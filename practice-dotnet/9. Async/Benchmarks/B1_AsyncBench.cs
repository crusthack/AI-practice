using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Running;
using Async.Demos;

BenchmarkRunner.Run<AsyncBenchmarks>();

[MemoryDiagnoser, ShortRunJob]
public class AsyncBenchmarks
{
    private static readonly AsyncCache<int, int> Cache = new();

    [Benchmark(Baseline = true)]
    public async Task<int> TaskFromResult() => await Task.FromResult(42);

    [Benchmark]
    public async ValueTask<int> ValueTaskFromResult() => await ValueTask.FromResult(42);

    [Benchmark]
    public async Task<int> AsyncCacheMiss()
    {
        var freshCache = new AsyncCache<int, int>();
        return await freshCache.GetOrAddAsync(1, async k => { await Task.Delay(0); return k; });
    }

    [Benchmark]
    public async Task<int> AsyncCacheHit()
    {
        return await Cache.GetOrAddAsync(99, async k => { await Task.Delay(0); return k; });
    }
}
