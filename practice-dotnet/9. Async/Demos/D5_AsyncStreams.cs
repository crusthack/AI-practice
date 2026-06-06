using System.Runtime.CompilerServices;

namespace Async.Demos;

static class D5_AsyncStreams
{
    public static async Task Run()
    {
        Print.Header("5. IAsyncEnumerable — 비동기 스트림");
        await ShowBasicAsyncStream();
        await ShowCancellableStream();
        await ShowAsyncLinq();
        await ShowRealWorldPattern();
        await ShowAsyncEnumeratorDirectly();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowBasicAsyncStream()
    {
        Print.Section("5-1. IAsyncEnumerable<T> 기본 — yield return");

        var total = 0;
        await foreach (var n in GenerateNumbers(1, 5))
        {
            total += n;
            Console.Write($" {n}");
        }
        Console.WriteLine();
        Print.Line($"  합계: {total}");

        // async 메서드에서 직접 반환
        await foreach (var line in ReadFakeFileAsync())
            Print.Line($"  파일 줄: {line}");

        static async IAsyncEnumerable<int> GenerateNumbers(int start, int count)
        {
            for (int i = start; i < start + count; i++)
            {
                await Task.Delay(1); // 비동기 IO 시뮬레이션
                yield return i;
            }
        }

        static async IAsyncEnumerable<string> ReadFakeFileAsync()
        {
            for (int i = 1; i <= 3; i++)
            {
                await Task.Delay(1);
                yield return $"줄 {i}: 내용";
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowCancellableStream()
    {
        Print.Section("5-2. 취소 가능한 스트림 — [EnumeratorCancellation]");

        using var cts = new CancellationTokenSource(100); // 100ms 후 취소

        var processed = new List<int>();
        try
        {
            await foreach (var item in SlowStream(cts.Token))
            {
                processed.Add(item);
                Print.Line($"  처리: {item}");
            }
        }
        catch (OperationCanceledException)
        {
            Print.Line($"  스트림 취소됨  (처리 수: {processed.Count})");
        }

        // WithCancellation — await foreach에 토큰 전달
        using var cts2 = new CancellationTokenSource(50);
        var count = 0;
        try
        {
            await foreach (var _ in SlowStream(default)
                                     .WithCancellation(cts2.Token))
                count++;
        }
        catch (OperationCanceledException)
        {
            Print.Line($"  WithCancellation 취소 (count={count})");
        }

        static async IAsyncEnumerable<int> SlowStream(
            [EnumeratorCancellation] CancellationToken ct = default)
        {
            for (int i = 0; i < 20; i++)
            {
                ct.ThrowIfCancellationRequested();
                await Task.Delay(20, ct);
                yield return i;
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowAsyncLinq()
    {
        Print.Section("5-3. 비동기 스트림 + LINQ");

        // System.Linq.Async 없이 수동 처리
        var results = new List<string>();
        await foreach (var item in FilterAndMapAsync(GetItemsAsync(), x => x % 2 == 0, x => $"짝수_{x}"))
            results.Add(item);
        Print.Items(results, "  결과: ");

        // ToListAsync 직접 구현
        var list = await ToListAsync(GetItemsAsync());
        Print.Line($"  ToList 길이: {list.Count}");

        static async IAsyncEnumerable<int> GetItemsAsync()
        {
            foreach (int i in Enumerable.Range(1, 6))
            {
                await Task.Delay(1);
                yield return i;
            }
        }

        static async IAsyncEnumerable<TOut> FilterAndMapAsync<T, TOut>(
            IAsyncEnumerable<T> source,
            Func<T, bool> predicate,
            Func<T, TOut> selector)
        {
            await foreach (var item in source)
                if (predicate(item))
                    yield return selector(item);
        }

        static async Task<List<T>> ToListAsync<T>(IAsyncEnumerable<T> source)
        {
            var result = new List<T>();
            await foreach (var item in source)
                result.Add(item);
            return result;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowRealWorldPattern()
    {
        Print.Section("5-4. 실제 사례 — 페이지네이션 스트림");

        // DB/API 결과를 페이지 단위로 스트리밍
        var totalFetched = 0;
        await foreach (var item in GetPagedResultsAsync(pageSize: 3, totalItems: 10))
        {
            totalFetched++;
            // 처리 중 메모리에 전부 올리지 않음
        }
        Print.Line($"  페이지네이션 스트림 총 항목: {totalFetched}");

        // 실시간 이벤트 스트림 시뮬레이션
        using var cts = new CancellationTokenSource(120);
        var eventCount = 0;
        try
        {
            await foreach (var evt in LiveEventStream(cts.Token))
            {
                eventCount++;
                if (eventCount == 1) Print.Line($"  첫 이벤트: {evt}");
            }
        }
        catch (OperationCanceledException) { }
        Print.Line($"  총 이벤트 수: {eventCount}");

        static async IAsyncEnumerable<string> GetPagedResultsAsync(int pageSize, int totalItems)
        {
            for (int page = 0; page * pageSize < totalItems; page++)
            {
                await Task.Delay(5); // 페이지 조회 시뮬레이션
                foreach (int i in Enumerable.Range(page * pageSize, Math.Min(pageSize, totalItems - page * pageSize)))
                    yield return $"item_{i}";
            }
        }

        static async IAsyncEnumerable<string> LiveEventStream(
            [EnumeratorCancellation] CancellationToken ct = default)
        {
            var random = new Random(42);
            while (!ct.IsCancellationRequested)
            {
                await Task.Delay(random.Next(10, 30), ct);
                yield return $"event_{random.Next(100)}";
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowAsyncEnumeratorDirectly()
    {
        Print.Section("5-5. IAsyncEnumerator 직접 사용");

        // await foreach 없이 MoveNextAsync 직접 호출 (예외 처리 세밀 제어 시)
        var enumerator = GetNumbersAsync().GetAsyncEnumerator();
        try
        {
            while (await enumerator.MoveNextAsync())
            {
                var value = enumerator.Current;
                Console.Write($" {value}");
                if (value >= 3) break; // 조기 종료
            }
            Console.WriteLine();
        }
        finally
        {
            await enumerator.DisposeAsync(); // 반드시 Dispose
        }

        static async IAsyncEnumerable<int> GetNumbersAsync()
        {
            for (int i = 1; i <= 5; i++)
            {
                await Task.Delay(1);
                yield return i;
            }
        }
    }
}
