using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;

// ───────────────────────────────────────────────────────
// CPU 바운드 — 배열 합산으로 병렬화 방법 비교
//
// Sequential       : 단일 스레드, 캐시 친화적
// Parallel.For     : 스레드 로컬 누산기 → lock 경합 최소화
// Parallel.ForEach : 파티셔너 기반, 컬렉션 지향 API
// PLINQ            : 선언적. 내부적으로 Parallel.For와 유사
//
// 핵심 질문: 병렬 오버헤드 손익분기점은 어디인가?
// ───────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class CpuBoundBench
{
    [Params(100_000, 5_000_000)]
    public int Size;

    private double[] _data = null!;

    [GlobalSetup]
    public void Setup()
    {
        _data = new double[Size];
        var rng = new Random(42);
        for (int i = 0; i < _data.Length; i++)
            _data[i] = rng.NextDouble();
    }

    // 단순 for 루프 — L1/L2 캐시 친화적, 오버헤드 없음
    [Benchmark(Baseline = true)]
    public double Sequential()
    {
        double sum = 0;
        for (int i = 0; i < _data.Length; i++)
            sum += _data[i];
        return sum;
    }

    // 스레드 로컬 누산 → 마지막에 한 번만 lock
    [Benchmark]
    public double ParallelFor()
    {
        double total = 0;
        var lockObj = new object();
        Parallel.For(
            fromInclusive: 0,
            toExclusive: _data.Length,
            localInit: () => 0.0,
            body: (i, _, local) => local + _data[i],
            localFinally: local => { lock (lockObj) total += local; }
        );
        return total;
    }

    // Parallel.ForEach — 배열을 파티셔너로 분할
    [Benchmark]
    public double ParallelForEach()
    {
        double total = 0;
        var lockObj = new object();
        Parallel.ForEach(
            source: System.Collections.Concurrent.Partitioner.Create(_data, loadBalance: true),
            localInit: () => 0.0,
            body: (chunk, _, local) => local + chunk,
            localFinally: local => { lock (lockObj) total += local; }
        );
        return total;
    }

    // PLINQ — 선언적 병렬
    [Benchmark]
    public double PlinqSum()
    {
        return _data.AsParallel().Sum();
    }
}
