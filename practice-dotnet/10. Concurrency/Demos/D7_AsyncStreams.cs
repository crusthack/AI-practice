using System.Runtime.CompilerServices;
using System.Threading.Channels;

namespace Demos;

static class D7_AsyncStreams
{
    public static async Task RunAsync()
    {
        Print.Header("7. Async Streams and Timers");

        await ShowAsyncIteratorAsync();
        await ShowPeriodicTimerAsync();
        await ShowChannelAsAsyncEnumerableAsync();
    }

    static async Task ShowAsyncIteratorAsync()
    {
        Print.Section("7-1. IAsyncEnumerable<T> - pull-based async sequence");

        await foreach (int item in GenerateAsync(5, TimeSpan.FromMilliseconds(10)))
            Console.Write($"{item} ");

        Console.WriteLine();
    }

    static async IAsyncEnumerable<int> GenerateAsync(
        int count,
        TimeSpan delay,
        [EnumeratorCancellation] CancellationToken ct = default)
    {
        for (int i = 0; i < count; i++)
        {
            await Task.Delay(delay, ct);
            yield return i;
        }
    }

    static async Task ShowPeriodicTimerAsync()
    {
        Print.Section("7-2. PeriodicTimer - async timer without callback reentrancy");

        using var timer = new PeriodicTimer(TimeSpan.FromMilliseconds(25));
        using var cts = new CancellationTokenSource(TimeSpan.FromMilliseconds(90));

        int ticks = 0;
        try
        {
            while (await timer.WaitForNextTickAsync(cts.Token))
            {
                ticks++;
                Console.Write($"tick{ticks} ");
            }
        }
        catch (OperationCanceledException)
        {
            Console.WriteLine($"\n    stopped after {ticks} ticks");
        }
    }

    static async Task ShowChannelAsAsyncEnumerableAsync()
    {
        Print.Section("7-3. ChannelReader.ReadAllAsync - push source, async stream consumer");

        var channel = Channel.CreateUnbounded<string>();

        var producer = Task.Run(async () =>
        {
            foreach (string item in new[] { "parse", "validate", "save" })
            {
                await channel.Writer.WriteAsync(item);
                await Task.Delay(5);
            }
            channel.Writer.Complete();
        });

        await foreach (string item in channel.Reader.ReadAllAsync())
            Console.WriteLine($"    step: {item}");

        await producer;
    }
}
