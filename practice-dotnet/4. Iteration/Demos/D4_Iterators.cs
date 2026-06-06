namespace Iteration.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 이터레이터 패턴:
//   IEnumerable<T>     → foreach 가능한 시퀀스
//   IEnumerator<T>     → 상태 머신: MoveNext() + Current
//   yield return       → 컴파일러가 상태 머신 자동 생성
//   yield break        → 시퀀스 종료
//   지연 평가(lazy)    → 실제 소비될 때까지 실행 안 됨
//   IAsyncEnumerable<T>→ await foreach 가능한 비동기 시퀀스
// ────────────────────────────────────────────────────────────────────────────
static class D4_Iterators
{
    public static void Run()
    {
        Print.Header("4. 이터레이터 (Iterators & yield)");

        ShowYieldBasics();
        ShowLazyEvaluation();
        ShowInfiniteSequences();
        ShowCustomIteratorClass();
        ShowYieldInStateMachine();
    }

    public static async Task RunAsync()
    {
        Print.Section("4-6. IAsyncEnumerable<T> — await foreach (비동기 스트림)");
        await ShowAsyncEnumerable();
    }

    // ── 4-1. yield return / yield break 기초 ─────────────────────────────────
    static void ShowYieldBasics()
    {
        Print.Section("4-1. yield return / yield break 기초");
        {
            Console.Write("    EvenNumbers(1,10): ");
            Print.Items(EvenNumbers(1, 10), "");

            Console.Write("    TakeWhile(<30):    ");
            Print.Items(EvenNumbers(1, 100).TakeWhile(x => x < 30), "");

            // yield break
            Console.Write("    UntilNegative:     ");
            Print.Items(UntilNegative(new[] { 3, 7, 2, -1, 5, 9 }), "");
        }
    }

    // 짝수 yield
    static IEnumerable<int> EvenNumbers(int start, int end)
    {
        for (int i = start; i <= end; i++)
            if (i % 2 == 0) yield return i;
    }

    // 음수를 만나면 중단
    static IEnumerable<int> UntilNegative(int[] arr)
    {
        foreach (int x in arr)
        {
            if (x < 0) yield break;
            yield return x;
        }
    }

    // ── 4-2. 지연 평가 (Lazy Evaluation) ─────────────────────────────────────
    static void ShowLazyEvaluation()
    {
        Print.Section("4-2. 지연 평가 — 소비할 때까지 실행 안 됨");
        {
            // 이터레이터는 ref/out 파라미터를 쓸 수 없으므로 클로저로 상태 공유
            var counter = new Counter();
            IEnumerable<int> lazy = ProduceWithCount(counter);

            Console.WriteLine($"    생성 직후 count={counter.Value}  (아직 실행 안 됨)");

            // 첫 번째 소비
            var first = lazy.First();
            Console.WriteLine($"    First() 후 count={counter.Value}  first={first}");

            // 전체 소비 (이터레이터는 매번 처음부터 다시 실행)
            _ = lazy.ToList();
            Console.WriteLine($"    ToList() 후 count={counter.Value}  (다시 처음부터 실행)");

            // 즉시 평가: ToList()로 결과를 메모리에 고정
            var eager = lazy.ToList();
            Console.WriteLine($"    eager.Count={eager.Count}  (완료 후 고정)");
        }
    }

    sealed class Counter { public int Value; }

    static IEnumerable<int> ProduceWithCount(Counter counter)
    {
        for (int i = 0; i < 5; i++)
        {
            counter.Value++;
            yield return i * 10;
        }
    }

    // ── 4-3. 무한 시퀀스 ─────────────────────────────────────────────────────
    static void ShowInfiniteSequences()
    {
        Print.Section("4-3. 무한 시퀀스 — yield로 끝없이 생성");
        {
            // 피보나치 무한 수열
            Console.Write("    Fibonacci 10개: ");
            Print.Items(FibonacciInfinite().Take(10), "");

            // 소수 무한 수열
            Console.Write("    소수 10개:      ");
            Print.Items(PrimesInfinite().Take(10), "");

            // Cycle (반복)
            Console.Write("    Cycle([A,B,C]) 7개: ");
            Print.Items(Cycle(new[] { "A", "B", "C" }).Take(7), "");
        }
    }

    static IEnumerable<long> FibonacciInfinite()
    {
        long a = 0, b = 1;
        while (true)
        {
            yield return a;
            (a, b) = (b, a + b);
        }
    }

    static IEnumerable<int> PrimesInfinite()
    {
        yield return 2;
        var known = new List<int> { 2 };
        for (int n = 3; ; n += 2)
        {
            if (known.All(p => n % p != 0))
            {
                known.Add(n);
                yield return n;
            }
        }
    }

    static IEnumerable<T> Cycle<T>(IEnumerable<T> source)
    {
        var list = source.ToList();
        while (true)
            foreach (var item in list)
                yield return item;
    }

    // ── 4-4. 커스텀 IEnumerable 클래스 ───────────────────────────────────────
    static void ShowCustomIteratorClass()
    {
        Print.Section("4-4. 커스텀 IEnumerable<T> + IEnumerator<T> 직접 구현");
        {
            var range = new NumberRange(1, 5);
            Console.Write("    NumberRange(1,5): ");
            Print.Items(range, "");

            // foreach 가능
            int sum = 0;
            foreach (int n in range) sum += n;
            Console.WriteLine($"    Sum={sum}");

            // LINQ 연산 가능 (IEnumerable<T> 구현)
            Console.Write("    짝수만:           ");
            Print.Items(range.Where(x => x % 2 == 0), "");

            // 독립적인 열거자
            var e1 = range.GetEnumerator();
            var e2 = range.GetEnumerator();
            e1.MoveNext(); e1.MoveNext(); e2.MoveNext();
            Console.WriteLine($"    e1.Current={e1.Current}  e2.Current={e2.Current}  (독립 상태)");
            e1.Dispose(); e2.Dispose();
        }
    }

    // ── 4-5. yield 상태 머신 이해 ────────────────────────────────────────────
    static void ShowYieldInStateMachine()
    {
        Print.Section("4-5. yield 상태 머신 — 실행 흐름 시각화");
        {
            Console.WriteLine("    StateMachineDemo 결과:");
            foreach (var step in StateMachineDemo())
                Console.WriteLine($"      {step}");
        }
    }

    static IEnumerable<string> StateMachineDemo()
    {
        Console.WriteLine("      [상태 0: 시작]");
        yield return "첫 번째 yield";

        Console.WriteLine("      [상태 1: 재개]");
        for (int i = 0; i < 2; i++)
        {
            Console.WriteLine($"      [상태 1.{i}: 루프 내부]");
            yield return $"루프 yield {i}";
        }

        Console.WriteLine("      [상태 2: 마지막]");
        yield return "마지막 yield";
        Console.WriteLine("      [종료]");
    }

    // ── 4-6. IAsyncEnumerable<T> ─────────────────────────────────────────────
    static async Task ShowAsyncEnumerable()
    {
        // await foreach: 각 항목을 비동기로 생산
        int totalBytes = 0;
        Console.Write("    스트림 수신: ");
        await foreach (var chunk in SimulateDataStream(count: 5, delayMs: 5))
        {
            Console.Write($"[{chunk.Length}B] ");
            totalBytes += chunk.Length;
        }
        Console.WriteLine($"\n    총 {totalBytes}바이트 수신");

        // ConfigureAwait on async enumerable
        await foreach (var n in CountAsync(3).ConfigureAwait(false))
            Console.Write($"    ConfigureAwait({n}) ");
        Console.WriteLine();
    }

    static async IAsyncEnumerable<byte[]> SimulateDataStream(int count, int delayMs)
    {
        var rng = new Random(42);
        for (int i = 0; i < count; i++)
        {
            await Task.Delay(delayMs);
            var chunk = new byte[rng.Next(4, 16)];
            rng.NextBytes(chunk);
            yield return chunk;
        }
    }

    static async IAsyncEnumerable<int> CountAsync(int n)
    {
        for (int i = 1; i <= n; i++)
        {
            await Task.Delay(1);
            yield return i;
        }
    }
}

// ── 커스텀 IEnumerable<T> + IEnumerator<T> 구현 ────────────────────────────

// IEnumerable<T>: foreach에 사용할 수 있게 하는 인터페이스
//                 GetEnumerator()를 구현해야 함
public sealed class NumberRange : IEnumerable<int>
{
    private readonly int _start, _end;

    public NumberRange(int start, int end)
    {
        _start = start;
        _end   = end;
    }

    // 호출될 때마다 새 열거자 반환 → 다중 독립 열거 지원
    public IEnumerator<int> GetEnumerator()          => new NumberRangeEnumerator(_start, _end);
    System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() => GetEnumerator();
}

// IEnumerator<T>: 실제 순회 로직 + 상태를 담는 클래스
// IDisposable: using / foreach 완료 시 자동 호출
public sealed class NumberRangeEnumerator : IEnumerator<int>
{
    private readonly int _start, _end;
    private int _current;

    public NumberRangeEnumerator(int start, int end)
    {
        _start   = start;
        _end     = end;
        _current = start - 1;  // Reset 상태: Current 미정의
    }

    // Current: 현재 위치의 값
    public int Current => _current;
    object System.Collections.IEnumerator.Current => _current;

    // MoveNext: 다음 위치로 이동, 요소가 있으면 true
    public bool MoveNext()
    {
        if (_current >= _end) return false;
        _current++;
        return true;
    }

    // Reset: 처음 상태로 (선택적, LINQ에서 일반적으로 사용 안 함)
    public void Reset() => _current = _start - 1;

    // Dispose: 열거자가 관리하는 리소스 해제 (여기선 없음)
    public void Dispose() { }
}
