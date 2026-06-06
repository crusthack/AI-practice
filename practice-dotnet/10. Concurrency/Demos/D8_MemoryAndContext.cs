namespace Demos;

static class D8_MemoryAndContext
{
    static readonly AsyncLocal<string?> RequestId = new();

    public static async Task RunAsync()
    {
        Print.Header("8. Memory Model and Context Flow");

        ShowThreadLocalVsAsyncLocal();
        await ShowAsyncLocalFlowAsync();
        await ShowVolatileSignalAsync();
        ShowCompareExchangeLoop();
    }

    static void ShowThreadLocalVsAsyncLocal()
    {
        Print.Section("8-1. ThreadLocal<T> - value is tied to the physical thread");

        using var local = new ThreadLocal<int>(() => Environment.CurrentManagedThreadId);

        Parallel.For(0, 4, i =>
        {
            Console.WriteLine($"    work {i}: thread={Environment.CurrentManagedThreadId}, local={local.Value}");
        });
    }

    static async Task ShowAsyncLocalFlowAsync()
    {
        Print.Section("8-2. AsyncLocal<T> - logical async context flow");

        RequestId.Value = "REQ-42";
        Console.WriteLine($"    before await: {RequestId.Value}");
        await NestedAsync();

        var flow = ExecutionContext.SuppressFlow();
        Task suppressed = Task.Run(() =>
        {
            Console.WriteLine($"    suppressed Task.Run: {RequestId.Value ?? "(null)"}");
        });
        flow.Dispose();
        await suppressed;

        RequestId.Value = null;

        static async Task NestedAsync()
        {
            await Task.Delay(1);
            Console.WriteLine($"    after await : {RequestId.Value}");
        }
    }

    static async Task ShowVolatileSignalAsync()
    {
        Print.Section("8-3. Volatile.Read/Write - visible cross-thread signal");

        var state = new SharedState();

        Task worker = Task.Run(() =>
        {
            while (!Volatile.Read(ref state.Done))
                Thread.SpinWait(100);

            Console.WriteLine($"    worker observed value={state.Value}");
        });

        state.Value = 123;
        Volatile.Write(ref state.Done, true);

        await worker;
    }

    static void ShowCompareExchangeLoop()
    {
        Print.Section("8-4. CompareExchange loop - basic lock-free update");

        int value = 0;

        Parallel.For(0, 10_000, _ => Add(ref value, 1));

        Console.WriteLine($"    CAS sum: {value}");

        static void Add(ref int target, int delta)
        {
            while (true)
            {
                int snapshot = Volatile.Read(ref target);
                int next = snapshot + delta;
                int original = Interlocked.CompareExchange(ref target, next, snapshot);

                if (original == snapshot)
                    return;
            }
        }
    }

    sealed class SharedState
    {
        public int Value;
        public bool Done;
    }
}
