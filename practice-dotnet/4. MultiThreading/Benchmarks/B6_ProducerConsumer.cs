using System.Collections.Concurrent;
using System.Threading.Channels;
using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;

// ───────────────────────────────────────────────────────
// 생산자-소비자 처리량 — Channel vs ConcurrentQueue vs BlockingCollection
//
// ConcurrentQueue  : lock-free. 소비자가 직접 폴링해야 함.
//                    비면 SpinWait 또는 Thread.Yield로 CPU 양보.
//
// BlockingCollection: ConcurrentQueue 래퍼. Take()가 빈 큐에서 블록.
//                    GetConsumingEnumerable()으로 foreach 지원.
//                    동기 블로킹이라 스레드를 점유.
//
// Channel<T>       : 비동기. ReadAllAsync()에서 데이터 없으면 await 대기.
//                    스레드 블록 없이 I/O처럼 동작 → CPU 효율 최고.
// ───────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class ProducerConsumerBench
{
    [Params(100_000)]
    public int ItemCount;

    // Channel<T> — 비동기, SingleWriter/SingleReader 최적화 옵션
    [Benchmark(Baseline = true)]
    public async Task Channel_Unbounded()
    {
        var ch = Channel.CreateUnbounded<int>(new UnboundedChannelOptions
        {
            SingleWriter = true,
            SingleReader = true,
            AllowSynchronousContinuations = false
        });

        var producer = Task.Run(() =>
        {
            for (int i = 0; i < ItemCount; i++)
                ch.Writer.TryWrite(i);
            ch.Writer.Complete();
        });

        var consumer = Task.Run(async () =>
        {
            int count = 0;
            await foreach (var _ in ch.Reader.ReadAllAsync())
                count++;
            return count;
        });

        await Task.WhenAll(producer, consumer);
    }

    // ConcurrentQueue — lock-free FIFO, 소비자는 SpinWait 폴링
    [Benchmark]
    public async Task ConcurrentQueue_SpinWait()
    {
        var queue = new ConcurrentQueue<int>();
        int consumed = 0;

        var producer = Task.Run(() =>
        {
            for (int i = 0; i < ItemCount; i++)
                queue.Enqueue(i);
        });

        var consumer = Task.Run(() =>
        {
            var spinner = new SpinWait();
            while (consumed < ItemCount)
            {
                if (queue.TryDequeue(out _))
                {
                    consumed++;
                    spinner.Reset();
                }
                else
                {
                    // 비면 적응형 대기: 초반은 CPU 회전, 점차 Thread.Yield, Sleep(0/1)
                    spinner.SpinOnce();
                }
            }
        });

        await Task.WhenAll(producer, consumer);
    }

    // BlockingCollection — 동기 블로킹. 소비자가 스레드를 점유한 채 기다림
    [Benchmark]
    public async Task BlockingCollection_T()
    {
        // ItemCount만큼 용량 설정 → 생산자가 블록되지 않게
        using var bc = new BlockingCollection<int>(boundedCapacity: ItemCount);

        var producer = Task.Run(() =>
        {
            for (int i = 0; i < ItemCount; i++)
                bc.Add(i);
            bc.CompleteAdding();
        });

        var consumer = Task.Run(() =>
        {
            foreach (var _ in bc.GetConsumingEnumerable()) { }
        });

        await Task.WhenAll(producer, consumer);
    }
}
