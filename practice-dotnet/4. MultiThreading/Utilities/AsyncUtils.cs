namespace MultiThreading.Utilities;

public static class AsyncUtils
{
    // N개의 비동기 작업을 최대 maxConcurrency개씩 동시에 실행
    public static async Task<T[]> WhenAllThrottled<T>(
        IEnumerable<Func<Task<T>>> factories,
        int maxConcurrency)
    {
        var semaphore = new SemaphoreSlim(maxConcurrency, maxConcurrency);
        var tasks = factories.Select(async factory =>
        {
            await semaphore.WaitAsync();
            try { return await factory(); }
            finally { semaphore.Release(); }
        });
        return await Task.WhenAll(tasks);
    }

    // 실제 최대 동시 실행 수를 추적하는 래퍼
    public static async Task<(T[] Results, int MaxConcurrency)> WhenAllTracked<T>(
        IEnumerable<Func<Task<T>>> factories,
        int maxConcurrency)
    {
        var semaphore = new SemaphoreSlim(maxConcurrency, maxConcurrency);
        int current = 0, max = 0;
        var lockObj = new object();

        var tasks = factories.Select(async factory =>
        {
            await semaphore.WaitAsync();
            int c = Interlocked.Increment(ref current);
            lock (lockObj) max = Math.Max(max, c);
            try { return await factory(); }
            finally
            {
                Interlocked.Decrement(ref current);
                semaphore.Release();
            }
        });

        var results = await Task.WhenAll(tasks);
        return (results, max);
    }

    // 지수 백오프(exponential backoff) 기반 재시도
    public static async Task<T> RetryAsync<T>(
        Func<Task<T>> operation,
        int maxAttempts,
        TimeSpan initialDelay,
        CancellationToken ct = default)
    {
        for (int attempt = 1; attempt < maxAttempts; attempt++)
        {
            try { return await operation(); }
            catch
            {
                var delay = TimeSpan.FromMilliseconds(
                    initialDelay.TotalMilliseconds * Math.Pow(2, attempt - 1));
                await Task.Delay(delay, ct);
            }
        }
        return await operation(); // 마지막 시도 — 예외 전파
    }
}
