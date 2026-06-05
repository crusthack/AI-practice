using System.Threading.Channels;
using MultiThreading.Utilities;

namespace MultiThreading.Tests;

public class AsyncUtilsTests
{
    [Fact]
    public async Task WhenAllThrottled_LimitsConcurrency()
    {
        const int limit = 3;
        int current = 0, maxObserved = 0;
        var lockObj = new object();

        var factories = Enumerable.Range(0, 12).Select<int, Func<Task<int>>>(i => async () =>
        {
            int c = Interlocked.Increment(ref current);
            lock (lockObj) maxObserved = Math.Max(maxObserved, c);
            await Task.Delay(20);
            Interlocked.Decrement(ref current);
            return i;
        });

        var results = await AsyncUtils.WhenAllThrottled(factories, limit);

        Assert.Equal(12, results.Length);
        Assert.True(maxObserved <= limit,
            $"최대 동시 실행 수 {maxObserved}가 제한({limit})을 초과");
    }

    [Fact]
    public async Task WhenAllThrottled_DeliversAllResults()
    {
        var factories = Enumerable.Range(0, 10).Select<int, Func<Task<int>>>(i =>
            () => Task.FromResult(i * i));

        var results = await AsyncUtils.WhenAllThrottled(factories, maxConcurrency: 4);

        Assert.Equal(10, results.Length);
        Assert.Equal(Enumerable.Range(0, 10).Select(i => i * i).Sum(), results.Sum());
    }

    [Fact]
    public async Task RetryAsync_SucceedsOnFirstAttempt()
    {
        int attempts = 0;
        var result = await AsyncUtils.RetryAsync(
            () => { attempts++; return Task.FromResult(42); },
            maxAttempts: 3,
            initialDelay: TimeSpan.FromMilliseconds(1));

        Assert.Equal(42, result);
        Assert.Equal(1, attempts);
    }

    [Fact]
    public async Task RetryAsync_RetriesOnFailure()
    {
        int attempts = 0;
        var result = await AsyncUtils.RetryAsync(
            () =>
            {
                attempts++;
                if (attempts < 3) throw new InvalidOperationException("일시적 오류");
                return Task.FromResult(99);
            },
            maxAttempts: 3,
            initialDelay: TimeSpan.FromMilliseconds(1));

        Assert.Equal(99, result);
        Assert.Equal(3, attempts);
    }
}

public class ChannelTests
{
    [Fact]
    public async Task Channel_Unbounded_DeliversAllItems()
    {
        const int count = 1_000;
        var ch = Channel.CreateUnbounded<int>();
        var received = new List<int>();

        var producer = Task.Run(async () =>
        {
            for (int i = 0; i < count; i++)
                await ch.Writer.WriteAsync(i);
            ch.Writer.Complete();
        });

        var consumer = Task.Run(async () =>
        {
            await foreach (var item in ch.Reader.ReadAllAsync())
                received.Add(item);
        });

        await Task.WhenAll(producer, consumer);

        Assert.Equal(count, received.Count);
        Assert.Equal(Enumerable.Range(0, count).Sum(), received.Sum());
    }

    [Fact]
    public async Task Channel_Bounded_AppliesBackpressure()
    {
        var ch = Channel.CreateBounded<int>(new BoundedChannelOptions(3)
        {
            FullMode = BoundedChannelFullMode.Wait
        });

        var producer = Task.Run(async () =>
        {
            for (int i = 0; i < 10; i++)
                await ch.Writer.WriteAsync(i);
            ch.Writer.Complete();
        });

        int consumed = 0;
        var consumer = Task.Run(async () =>
        {
            await foreach (var _ in ch.Reader.ReadAllAsync())
            {
                await Task.Delay(5); // 의도적으로 느린 소비자
                consumed++;
            }
        });

        await Task.WhenAll(producer, consumer);
        Assert.Equal(10, consumed);
    }

    [Fact]
    public async Task Interlocked_IsThreadSafe()
    {
        long counter = 0;
        await Task.WhenAll(Enumerable.Range(0, 100).Select(_ => Task.Run(() =>
        {
            for (int i = 0; i < 1_000; i++)
                Interlocked.Increment(ref counter);
        })));

        Assert.Equal(100L * 1_000L, counter);
    }
}
