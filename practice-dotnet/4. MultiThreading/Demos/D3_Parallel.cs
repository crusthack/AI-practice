namespace Demos;

// ───────────────────────────────────────────────────────
// Parallel 클래스 & PLINQ
//
// Parallel.For / ForEach / Invoke :
//   TPL(Task Parallel Library)이 제공하는 구조적 병렬 루프.
//   내부적으로 데이터를 파티셔닝하고 ThreadPool에 분배한다.
//
// PLINQ (Parallel LINQ):
//   기존 LINQ 체인에 AsParallel()을 추가하면 자동 병렬화.
//   선언적이고 간결하지만, 작은 컬렉션은 오버헤드가 더 크다.
// ───────────────────────────────────────────────────────
static class D3_Parallel
{
    public static async Task RunAsync()
    {
        Print.Header("3. Parallel 클래스 & PLINQ");

        double[] data = GenerateData(5_000_000);

        // ── 3-1. Parallel.For — 스레드 로컬 누산기 패턴
        Print.Section("3-1. Parallel.For — 스레드 로컬 누산기로 lock 경합 최소화");
        {
            var sw = System.Diagnostics.Stopwatch.StartNew();
            double total = 0;
            var lockObj = new object();
            Parallel.For(
                fromInclusive: 0,
                toExclusive: data.Length,
                localInit: () => 0.0,                            // 각 스레드의 로컬 초기값
                body: (i, _, local) => local + data[i],          // 로컬에만 누산 → lock 없음
                localFinally: local => { lock (lockObj) total += local; } // 마지막에 한 번만 lock
            );
            Console.WriteLine($"    합계: {total:F2}  ({sw.ElapsedMilliseconds}ms)");
        }

        // ── 3-2. Parallel.ForEach — 컬렉션 병렬 처리 + MaxDegreeOfParallelism
        Print.Section("3-2. Parallel.ForEach — 병렬 옵션으로 스레드 수 제한");
        {
            string[] urls = Enumerable.Range(0, 12).Select(i => $"url-{i}").ToArray();
            int processed = 0;
            Parallel.ForEach(
                urls,
                new ParallelOptions { MaxDegreeOfParallelism = 4 }, // 최대 4 스레드
                url =>
                {
                    // 실제로는 HTTP 요청 등 CPU 바운드 전처리
                    Interlocked.Increment(ref processed);
                }
            );
            Console.WriteLine($"    처리된 항목: {processed} / {urls.Length}");
        }

        // ── 3-3. Parallel.Invoke — 독립 작업 동시 실행
        Print.Section("3-3. Parallel.Invoke — 독립 작업 병렬 실행");
        {
            var sw = System.Diagnostics.Stopwatch.StartNew();
            Parallel.Invoke(
                () => { Thread.Sleep(80); Console.WriteLine("    작업 A 완료"); },
                () => { Thread.Sleep(60); Console.WriteLine("    작업 B 완료"); },
                () => { Thread.Sleep(70); Console.WriteLine("    작업 C 완료"); }
            );
            Console.WriteLine($"    전체: {sw.ElapsedMilliseconds}ms  (순차 실행이면 ~210ms)");
        }

        // ── 3-4. PLINQ — AsParallel()
        Print.Section("3-4. PLINQ — AsParallel() 선언적 병렬화");
        {
            int[] numbers = Enumerable.Range(0, 2_000_000).ToArray();

            var sw = System.Diagnostics.Stopwatch.StartNew();
            long seqSum = numbers.Where(x => x % 2 == 0).Select(x => (long)x).Sum();
            long seqMs  = sw.ElapsedMilliseconds;

            sw.Restart();
            long parSum = numbers.AsParallel()
                                 .Where(x => x % 2 == 0)
                                 .Select(x => (long)x)
                                 .Sum();
            long parMs  = sw.ElapsedMilliseconds;

            Console.WriteLine($"    순차 LINQ : {seqSum:N0}  ({seqMs}ms)");
            Console.WriteLine($"    PLINQ     : {parSum:N0}  ({parMs}ms)");
        }

        // ── 3-5. PLINQ — WithDegreeOfParallelism & AsOrdered
        Print.Section("3-5. PLINQ 제어 — WithDegreeOfParallelism & AsOrdered");
        {
            int[] src = [1, 2, 3, 4, 5, 6, 7, 8];

            int[] unordered = src.AsParallel()
                                 .WithDegreeOfParallelism(4)   // 최대 4코어 사용
                                 .Select(x => x * x)
                                 .ToArray();

            int[] ordered = src.AsParallel()
                               .AsOrdered()                    // 원본 순서 보장 (오버헤드 있음)
                               .Select(x => x * x)
                               .ToArray();

            Console.WriteLine($"    비순서: [{string.Join(", ", unordered)}]");
            Console.WriteLine($"    순서  : [{string.Join(", ", ordered)}]");
            Console.WriteLine("    AsOrdered() 없으면 병렬 실행 순서에 따라 결과 순서가 달라질 수 있음");
        }

        // ── 3-6. Parallel 주의사항
        Print.Section("3-6. 언제 병렬화하면 안 되는가");
        Console.WriteLine("    ✗ 작업량이 적을 때: 스레드 생성·동기화 오버헤드 > 병렬 이득");
        Console.WriteLine("    ✗ 공유 상태 변이: 경쟁 조건 발생 (lock 없이 counter++ 등)");
        Console.WriteLine("    ✗ 순서 의존 처리: 이전 결과가 다음 입력인 경우");
        Console.WriteLine("    ✗ I/O 바운드 작업: async/await + Task.WhenAll이 더 적합");
        Console.WriteLine("    ✓ CPU 바운드 + 데이터량 충분 + 각 항목이 독립적인 경우");
    }

    static double[] GenerateData(int size)
    {
        var rng = new Random(42);
        var arr = new double[size];
        for (int i = 0; i < size; i++) arr[i] = rng.NextDouble();
        return arr;
    }
}
