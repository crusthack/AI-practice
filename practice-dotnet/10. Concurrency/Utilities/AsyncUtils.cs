namespace MultiThreading.Utilities;

public static class AsyncUtils
{
    // Run asynchronous operations with a fixed concurrency limit.
    public static async Task<T[]> WhenAllThrottled<T>(
        IEnumerable<Func<Task<T>>> factories,
        int maxConcurrency,
        CancellationToken ct = default)
    {
        ArgumentNullException.ThrowIfNull(factories);
        ArgumentOutOfRangeException.ThrowIfLessThan(maxConcurrency, 1);

        using var semaphore = new SemaphoreSlim(maxConcurrency, maxConcurrency);
        var tasks = factories.Select(async factory =>
        {
            await semaphore.WaitAsync(ct);
            try { return await factory(); }
            finally { semaphore.Release(); }
        });

        return await Task.WhenAll(tasks);
    }

    // Run operations with throttling and report the observed peak concurrency.
    public static async Task<(T[] Results, int MaxConcurrency)> WhenAllTracked<T>(
        IEnumerable<Func<Task<T>>> factories,
        int maxConcurrency,
        CancellationToken ct = default)
    {
        ArgumentNullException.ThrowIfNull(factories);
        ArgumentOutOfRangeException.ThrowIfLessThan(maxConcurrency, 1);

        using var semaphore = new SemaphoreSlim(maxConcurrency, maxConcurrency);
        int current = 0, max = 0;
        var lockObj = new object();

        var tasks = factories.Select(async factory =>
        {
            await semaphore.WaitAsync(ct);
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

    // Retry with exponential backoff. Cancellation is never retried.
    public static async Task<T> RetryAsync<T>(
        Func<Task<T>> operation,
        int maxAttempts,
        TimeSpan initialDelay,
        CancellationToken ct = default)
    {
        ArgumentNullException.ThrowIfNull(operation);
        ArgumentOutOfRangeException.ThrowIfLessThan(maxAttempts, 1);
        ArgumentOutOfRangeException.ThrowIfLessThan(initialDelay, TimeSpan.Zero);

        for (int attempt = 1; attempt <= maxAttempts; attempt++)
        {
            ct.ThrowIfCancellationRequested();

            try { return await operation(); }
            catch (OperationCanceledException)
            {
                throw;
            }
            catch when (attempt < maxAttempts)
            {
                var delay = TimeSpan.FromMilliseconds(
                    initialDelay.TotalMilliseconds * Math.Pow(2, attempt - 1));
                await Task.Delay(delay, ct);
            }
        }

        throw new InvalidOperationException("Retry loop ended unexpectedly.");
    }
}
