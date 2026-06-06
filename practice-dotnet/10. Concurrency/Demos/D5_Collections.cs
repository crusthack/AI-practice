using System.Collections.Concurrent;
using System.Threading.Channels;

namespace Demos;

// ───────────────────────────────────────────────────────
// 동시성 컬렉션 (System.Collections.Concurrent)
// 채널 (System.Threading.Channels)
//
// ConcurrentDictionary : lock-free 해시맵.
//                        AddOrUpdate / GetOrAdd 원자적 실행.
// ConcurrentQueue<T>   : lock-free FIFO. Enqueue/TryDequeue.
// ConcurrentStack<T>   : lock-free LIFO.
// ConcurrentBag<T>     : 순서 없는 lock-free 컬렉션.
// BlockingCollection<T>: 생산자-소비자 래퍼. 용량 제한 + 블로킹.
//                        내부 스토어 교체 가능(기본: ConcurrentQueue).
// Channel<T>           : 현대적 비동기 파이프라인. WaitToReadAsync/WriteAsync.
//                        Bounded: 백프레셔(backpressure) 자동 적용.
// ───────────────────────────────────────────────────────
static class D5_Collections
{
    public static async Task RunAsync()
    {
        Print.Header("5. 동시성 컬렉션 & Channel");

        // ── 5-1. ConcurrentDictionary
        Print.Section("5-1. ConcurrentDictionary<K,V> — 원자적 AddOrUpdate / GetOrAdd");
        {
            var dict = new ConcurrentDictionary<string, int>();

            Parallel.For(0, 10_000, i =>
            {
                string key = $"bucket{i % 5}";
                // AddOrUpdate: 키 없으면 추가, 있으면 업데이트 — 원자적
                dict.AddOrUpdate(key, addValue: 1, updateValueFactory: (_, v) => v + 1);
            });

            Console.WriteLine($"    버킷 수: {dict.Count}  합계: {dict.Values.Sum()}  (기대: 10000)");

            // GetOrAdd: 없으면 팩토리 호출 후 추가 (lazy initialization에 유용)
            var conn = dict.GetOrAdd("new-key", key =>
            {
                Console.WriteLine($"    GetOrAdd: '{key}' 초기화 중...");
                return 0;
            });
        }

        // ── 5-2. ConcurrentQueue / Stack / Bag
        Print.Section("5-2. ConcurrentQueue / Stack / Bag 비교");
        {
            var queue = new ConcurrentQueue<int>(); // FIFO
            var stack = new ConcurrentStack<int>(); // LIFO
            var bag   = new ConcurrentBag<int>();   // 순서 없음 (스레드 선호)

            for (int i = 1; i <= 5; i++) { queue.Enqueue(i); stack.Push(i); bag.Add(i); }

            queue.TryDequeue(out int qFirst);
            stack.TryPop(out int sFirst);
            bag.TryTake(out int bFirst);

            Console.WriteLine($"    Queue(FIFO): {qFirst}  Stack(LIFO): {sFirst}  Bag(임의): {bFirst}");
            Console.WriteLine("    Bag: 스레드별 로컬 리스트 → 자신이 넣은 걸 먼저 꺼냄 (work-stealing)");
        }

        // ── 5-3. BlockingCollection — 생산자-소비자 (동기 블로킹)
        Print.Section("5-3. BlockingCollection<T> — 용량 제한 + 동기 블로킹");
        {
            using var bc = new BlockingCollection<int>(boundedCapacity: 4);

            var producer = Task.Run(() =>
            {
                for (int i = 0; i < 8; i++)
                {
                    bc.Add(i);              // 꽉 차면 블록 (백프레셔)
                    Console.Write($"P{i} ");
                }
                bc.CompleteAdding();        // 생산 종료 신호
            });

            var consumer = Task.Run(() =>
            {
                foreach (int item in bc.GetConsumingEnumerable()) // 비면 블록
                {
                    Thread.Sleep(15);       // 소비가 더 느림 → 생산자가 멈춤
                    Console.Write($"C{item} ");
                }
            });

            await Task.WhenAll(producer, consumer);
            Console.WriteLine();
        }

        // ── 5-4. Channel<T> Unbounded — 비동기 파이프라인
        Print.Section("5-4. Channel<T> Unbounded — 비동기 생산자-소비자");
        {
            var ch = Channel.CreateUnbounded<string>(new UnboundedChannelOptions
            {
                SingleReader = true,
                SingleWriter = true,
                AllowSynchronousContinuations = false
            });

            var producer = Task.Run(async () =>
            {
                string[] items = ["사과", "바나나", "체리", "대추", "엘더베리"];
                foreach (var item in items)
                {
                    await ch.Writer.WriteAsync(item);
                    Console.Write($"[생산:{item}] ");
                    await Task.Delay(5);
                }
                ch.Writer.Complete(); // 채널 닫기
            });

            var consumer = Task.Run(async () =>
            {
                await foreach (var item in ch.Reader.ReadAllAsync())
                {
                    Console.Write($"[소비:{item}] ");
                }
            });

            await Task.WhenAll(producer, consumer);
            Console.WriteLine();
        }

        // ── 5-5. Channel<T> Bounded — 백프레셔 자동 적용
        Print.Section("5-5. Channel<T> Bounded — 백프레셔 (생산자 속도 자동 조절)");
        {
            var ch = Channel.CreateBounded<int>(new BoundedChannelOptions(capacity: 3)
            {
                // Wait: 꽉 차면 생산자의 WriteAsync가 await 상태로 대기
                FullMode = BoundedChannelFullMode.Wait,
                SingleReader = true,
                SingleWriter = true
            });

            var producer = Task.Run(async () =>
            {
                for (int i = 0; i < 9; i++)
                {
                    await ch.Writer.WriteAsync(i); // 3개 찬 후 소비될 때까지 대기
                    Console.Write($"P{i} ");
                }
                ch.Writer.Complete();
            });

            var consumer = Task.Run(async () =>
            {
                await foreach (var i in ch.Reader.ReadAllAsync())
                {
                    await Task.Delay(25); // 소비가 느림
                    Console.Write($"C{i} ");
                }
            });

            await Task.WhenAll(producer, consumer);
            Console.WriteLine("\n    생산자가 소비자 속도에 맞춰 자동으로 멈춤 (백프레셔)");
        }

        // ── 5-6. 멀티 생산자 - 멀티 소비자 패턴
        Print.Section("5-6. Channel — 멀티 생산자 × 멀티 소비자");
        {
            var ch = Channel.CreateUnbounded<int>();
            int consumed = 0;

            // 생산자 3개
            var producers = Enumerable.Range(0, 3).Select(p => Task.Run(async () =>
            {
                for (int i = 0; i < 5; i++)
                    await ch.Writer.WriteAsync(p * 100 + i);
            })).ToArray();

            // 생산자 전원 완료 후 채널 닫기
            _ = Task.WhenAll(producers).ContinueWith(_ => ch.Writer.Complete());

            // 소비자 2개
            var consumers = Enumerable.Range(0, 2).Select(_ => Task.Run(async () =>
            {
                await foreach (var item in ch.Reader.ReadAllAsync())
                    Interlocked.Increment(ref consumed);
            })).ToArray();

            await Task.WhenAll(producers);
            await Task.WhenAll(consumers);
            Console.WriteLine($"    생산자 3 × 5 = {consumed}개 소비  (소비자 2개 분담)");
        }
    }
}
