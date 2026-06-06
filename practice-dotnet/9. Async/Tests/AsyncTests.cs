using Async.Demos;
using Xunit;

public class TaskTests
{
    [Fact] public async Task Task_FromResult_IsComplete()
    {
        var t = Task.FromResult(42);
        Assert.True(t.IsCompleted);
        Assert.Equal(42, await t);
    }

    [Fact] public async Task Task_WhenAll_CollectsResults()
    {
        var results = await Task.WhenAll(
            Task.FromResult(1),
            Task.FromResult(2),
            Task.FromResult(3));
        Assert.Equal([1, 2, 3], results);
    }

    [Fact] public async Task Task_WhenAny_ReturnsFastest()
    {
        var fast = Task.FromResult(1);
        var slow = Task.Delay(500).ContinueWith(_ => 2);
        var first = await await Task.WhenAny(fast, slow);
        Assert.Equal(1, first);
    }

    [Fact] public async Task Task_Exception_Propagates()
    {
        await Assert.ThrowsAsync<InvalidOperationException>(async () =>
            await Task.Run(() => throw new InvalidOperationException("test")));
    }

    [Fact] public async Task TaskCompletionSource_SetResult_Works()
    {
        var tcs = new TaskCompletionSource<string>();
        _ = Task.Run(() => tcs.SetResult("done"));
        Assert.Equal("done", await tcs.Task);
    }
}

public class ValueTaskTests
{
    [Fact] public async Task ValueTask_FromResult_NoAllocation()
    {
        var vt = ValueTask.FromResult(99);
        Assert.True(vt.IsCompleted);
        Assert.Equal(99, await vt);
    }

    [Fact] public async Task AsyncCache_CachesResult()
    {
        var cache = new AsyncCache<string, int>();
        int callCount = 0;

        var v1 = await cache.GetOrAddAsync("k", async _ => { callCount++; await Task.Delay(1); return 42; });
        var v2 = await cache.GetOrAddAsync("k", async _ => { callCount++; await Task.Delay(1); return 99; });

        Assert.Equal(42, v1);
        Assert.Equal(42, v2);
        Assert.Equal(1, callCount); // 두 번째 호출은 캐시에서
    }
}

public class CancellationTests
{
    [Fact] public async Task CancellationToken_Cancelled_ThrowsOCE()
    {
        using var cts = new CancellationTokenSource();
        cts.Cancel();
        await Assert.ThrowsAsync<TaskCanceledException>(async () =>
            await Task.Delay(1000, cts.Token));
    }

    [Fact] public async Task LinkedToken_ParentCancel_PropagatesChild()
    {
        using var parent = new CancellationTokenSource();
        using var linked = CancellationTokenSource.CreateLinkedTokenSource(parent.Token);
        parent.Cancel();
        Assert.True(linked.Token.IsCancellationRequested);
        await Task.CompletedTask;
    }

    [Fact] public async Task TimeoutCts_CancelsAfterDelay()
    {
        using var cts = new CancellationTokenSource(50);
        await Assert.ThrowsAsync<TaskCanceledException>(async () =>
            await Task.Delay(500, cts.Token));
    }

    [Fact] public void CancellationToken_Register_Fires()
    {
        using var cts = new CancellationTokenSource();
        bool fired = false;
        cts.Token.Register(() => fired = true);
        cts.Cancel();
        Assert.True(fired);
    }
}

public class AsyncStreamTests
{
    [Fact] public async Task AsyncStream_YieldsAll()
    {
        var items = new List<int>();
        await foreach (var n in Numbers(1, 5))
            items.Add(n);
        Assert.Equal([1, 2, 3, 4, 5], items);

        static async IAsyncEnumerable<int> Numbers(int start, int count)
        {
            for (int i = start; i < start + count; i++)
            {
                await Task.Yield();
                yield return i;
            }
        }
    }

    [Fact] public async Task AsyncStream_WithCancellation_Stops()
    {
        using var cts = new CancellationTokenSource();
        var count = 0;
        cts.Cancel();

        await Assert.ThrowsAsync<OperationCanceledException>(async () =>
        {
            await foreach (var _ in Infinite().WithCancellation(cts.Token))
                count++;
        });

        static async IAsyncEnumerable<int> Infinite(
            [System.Runtime.CompilerServices.EnumeratorCancellation] CancellationToken ct = default)
        {
            var i = 0;
            while (true)
            {
                ct.ThrowIfCancellationRequested();
                await Task.Delay(1, ct);
                yield return i++;
            }
        }
    }
}
