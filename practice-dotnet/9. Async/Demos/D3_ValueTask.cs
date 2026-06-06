using System.Runtime.CompilerServices;

namespace Async.Demos;

static class D3_ValueTask
{
    public static async Task Run()
    {
        Print.Header("3. ValueTask · IValueTaskSource — 할당 최소화 비동기");
        await ShowValueTaskBasics();
        await ShowValueTaskUseCases();
        await ShowIValueTaskSource();
        await ShowValueTaskPitfalls();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowValueTaskBasics()
    {
        Print.Section("3-1. ValueTask<T> vs Task<T>");

        // ValueTask<T>: 동기 완료 시 Task 객체 할당 없음
        var sw = System.Diagnostics.Stopwatch.StartNew();

        // Task<T> — 항상 힙 할당
        for (int i = 0; i < 1000; i++)
            await TaskMethod(i);
        long taskMs = sw.ElapsedMilliseconds;

        sw.Restart();
        // ValueTask<T> — 캐시 hit 시 할당 없음
        for (int i = 0; i < 1000; i++)
            await ValueTaskMethod(i);
        long vtMs = sw.ElapsedMilliseconds;

        Print.Line($"  Task:      {taskMs}ms");
        Print.Line($"  ValueTask: {vtMs}ms  (캐시 hit 시 더 빠름)");

        // ValueTask에서 Task로 변환
        ValueTask<int> vt = ValueTask.FromResult(42);
        Task<int> t = vt.AsTask();
        Print.Line($"  AsTask: {await t}");

        static async Task TaskMethod(int n)
        {
            if (n < 0) await Task.Delay(0); // 거의 항상 동기 완료
        }

        static ValueTask ValueTaskMethod(int n)
        {
            if (n < 0) return new ValueTask(Task.Delay(0));
            return ValueTask.CompletedTask; // 할당 없음
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowValueTaskUseCases()
    {
        Print.Section("3-2. ValueTask 적합 사례 — 캐시 · 풀링");

        var cache = new AsyncCache<string, string>();

        // 첫 번째 호출: 실제 IO (Task 할당 있음)
        string val1 = await cache.GetOrAddAsync("key1", async k =>
        {
            await Task.Delay(1);
            return $"value_of_{k}";
        });

        // 두 번째 호출: 캐시 hit (ValueTask.FromResult — 할당 없음)
        string val2 = await cache.GetOrAddAsync("key1", async k =>
        {
            await Task.Delay(1);
            return "should_not_compute";
        });

        Print.Line($"  첫 호출:   {val1}");
        Print.Line($"  캐시 hit:  {val2}  (동일)");
        Print.Line($"  캐시 동일: {val1 == val2}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowIValueTaskSource()
    {
        Print.Section("3-3. ManualResetValueTaskSourceCore — 재사용 가능 소스");

        // IValueTaskSource 직접 구현은 고급이지만 System.Threading.Channels에서 사용
        // 여기서는 동작 원리를 시뮬레이션

        var signal = new AsyncAutoResetEvent();
        var sw = System.Diagnostics.Stopwatch.StartNew();

        // 생산자
        _ = Task.Run(async () =>
        {
            await Task.Delay(20);
            signal.Set();
        });

        // 소비자
        await signal.WaitAsync();
        Print.Line($"  AsyncAutoResetEvent 대기: {sw.ElapsedMilliseconds}ms");

        // 재사용
        _ = Task.Run(async () => { await Task.Delay(10); signal.Set(); });
        await signal.WaitAsync();
        Print.Line($"  두 번째 Wait: 완료");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowValueTaskPitfalls()
    {
        Print.Section("3-4. ValueTask 주의사항");

        Print.Line("  주의 1: ValueTask는 한 번만 await 가능 (다중 await = UB)");
        // 안전: AsTask()로 변환 후 다중 await
        var vt = GetDataAsync();
        var t  = vt.AsTask();     // 변환 후 여러 번 await 가능
        await t;
        await t;  // Task는 여러 번 await 가능
        Print.Line("  AsTask() 후 다중 await: 안전");

        Print.Line("  주의 2: ValueTask를 캐싱/저장하지 말 것");
        Print.Line("  주의 3: 풀링된 IValueTaskSource 결과는 await 직후 바로 소비");

        // ValueTask 올바른 사용 판단 기준
        Print.Line("  사용 기준:");
        Print.Line("    • 동기 완료 경로가 열 경로(hot path)인 경우 ← O");
        Print.Line("    • 항상 비동기 완료인 경우 ← Task가 더 단순");

        static ValueTask<int> GetDataAsync() => ValueTask.FromResult(42);
    }
}

// ── 헬퍼 클래스 ───────────────────────────────────────────────────────────────

public sealed class AsyncCache<TKey, TValue> where TKey : notnull
{
    private readonly Dictionary<TKey, TValue> _store = [];

    public ValueTask<TValue> GetOrAddAsync(TKey key, Func<TKey, Task<TValue>> factory)
    {
        if (_store.TryGetValue(key, out var cached))
            return ValueTask.FromResult(cached);     // 할당 없는 동기 반환
        return new ValueTask<TValue>(FetchAsync(key, factory));
    }

    private async Task<TValue> FetchAsync(TKey key, Func<TKey, Task<TValue>> factory)
    {
        var value = await factory(key);
        _store[key] = value;
        return value;
    }
}

sealed class AsyncAutoResetEvent
{
    private TaskCompletionSource _tcs = new(TaskCreationOptions.RunContinuationsAsynchronously);

    public void Set()
    {
        var old = Interlocked.Exchange(ref _tcs, new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously));
        old.TrySetResult();
    }

    public Task WaitAsync() => _tcs.Task;
}
