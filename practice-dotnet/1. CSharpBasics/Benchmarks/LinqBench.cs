using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using CSharpBasics;

// ─────────────────────────────────────────────────────
// LINQ vs 명령형 루프 vs Span<T> 성능 비교
//
// LINQ       : 선언적. 내부 가상 디스패치·델리게이트 호출 오버헤드.
// 명령형 루프: JIT 인라인·언롤 최적화 가능. 가장 제어 가능.
// Span<T>    : 배열 직접 접근. 범위 체크 제거 힌트.
// ─────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class LinqBench
{
    [Params(100, 10_000, 1_000_000)]
    public int Size;

    private int[] _data = null!;

    [GlobalSetup]
    public void Setup()
    {
        var rng = new Random(42);
        _data = Enumerable.Range(0, Size).Select(_ => rng.Next(0, 1000)).ToArray();
    }

    [Benchmark(Baseline = true)]
    public long LinqWhereSum() =>
        _data.Where(x => x % 2 == 0).Select(x => (long)x).Sum();

    [Benchmark]
    public long ImperativeLoop()
    {
        long sum = 0;
        for (int i = 0; i < _data.Length; i++)
            if (_data[i] % 2 == 0) sum += _data[i];
        return sum;
    }

    [Benchmark]
    public long SpanLoop()
    {
        long sum = 0;
        foreach (int x in _data.AsSpan())
            if (x % 2 == 0) sum += x;
        return sum;
    }

    [Benchmark]
    public int LinqPrimeCount() => _data.Count(NumberUtils.IsPrime);

    [Benchmark]
    public int ImperativePrimeCount()
    {
        int count = 0;
        for (int i = 0; i < _data.Length; i++)
            if (NumberUtils.IsPrime(_data[i])) count++;
        return count;
    }
}
