using System.Threading.Channels;

namespace Demos;

static class D6_AdvancedChannels
{
    public static async Task RunAsync()
    {
        Print.Header("6. Advanced Channel Patterns");

        await ShowBoundedFullModesAsync();
        await ShowCompletionWithErrorAsync();
        await ShowFanOutWorkQueueAsync();
        await ShowFanInMergeAsync();
    }

    static async Task ShowBoundedFullModesAsync()
    {
        Print.Section("6-1. BoundedChannelFullMode - drop policy");

        foreach (var mode in new[]
                 {
                     BoundedChannelFullMode.DropOldest,
                     BoundedChannelFullMode.DropNewest,
                     BoundedChannelFullMode.DropWrite
                 })
        {
            var channel = Channel.CreateBounded<int>(new BoundedChannelOptions(3)
            {
                FullMode = mode,
                SingleReader = true,
                SingleWriter = true
            });

            for (int i = 0; i < 8; i++)
                channel.Writer.TryWrite(i);
            channel.Writer.Complete();

            var received = new List<int>();
            await foreach (int item in channel.Reader.ReadAllAsync())
                received.Add(item);

            Console.WriteLine($"    {mode,-10}: [{string.Join(", ", received)}]");
        }

        Console.WriteLine("    Wait mode blocks producers; drop modes keep producers moving by losing data.");
    }

    static async Task ShowCompletionWithErrorAsync()
    {
        Print.Section("6-2. Complete(exception) - propagate producer failure");

        var channel = Channel.CreateUnbounded<int>();

        _ = Task.Run(async () =>
        {
            await channel.Writer.WriteAsync(1);
            await channel.Writer.WriteAsync(2);
            channel.Writer.Complete(new InvalidOperationException("producer failed"));
        });

        try
        {
            await foreach (int item in channel.Reader.ReadAllAsync())
                Console.WriteLine($"    read: {item}");
        }
        catch (InvalidOperationException ex)
        {
            Console.WriteLine($"    consumer observed: {ex.Message}");
        }
    }

    static async Task ShowFanOutWorkQueueAsync()
    {
        Print.Section("6-3. Fan-out - one queue, many workers");

        var channel = Channel.CreateUnbounded<int>();
        int completed = 0;

        var workers = Enumerable.Range(0, 3).Select(workerId => Task.Run(async () =>
        {
            await foreach (int job in channel.Reader.ReadAllAsync())
            {
                await Task.Delay(10);
                Interlocked.Increment(ref completed);
                Console.Write($"W{workerId}:{job} ");
            }
        })).ToArray();

        for (int job = 0; job < 12; job++)
            await channel.Writer.WriteAsync(job);
        channel.Writer.Complete();

        await Task.WhenAll(workers);
        Console.WriteLine($"\n    completed jobs: {completed}");
    }

    static async Task ShowFanInMergeAsync()
    {
        Print.Section("6-4. Fan-in - merge many producers into one reader");

        var channel = Channel.CreateUnbounded<string>();

        Task ProducerAsync(string name) => Task.Run(async () =>
        {
            for (int i = 0; i < 3; i++)
            {
                await Task.Delay(5 + i * 5);
                await channel.Writer.WriteAsync($"{name}{i}");
            }
        });

        Task[] producers =
        [
            ProducerAsync("A"),
            ProducerAsync("B"),
            ProducerAsync("C")
        ];

        _ = Task.WhenAll(producers).ContinueWith(_ => channel.Writer.Complete());

        var merged = new List<string>();
        await foreach (string item in channel.Reader.ReadAllAsync())
            merged.Add(item);

        await Task.WhenAll(producers);
        Console.WriteLine($"    merged: [{string.Join(", ", merged)}]");
    }
}
