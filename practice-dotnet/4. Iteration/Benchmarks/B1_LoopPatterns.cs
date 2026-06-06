using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;

namespace IterBench;

// ── 루프 패턴 벤치마크 ────────────────────────────────────────────────────
// 목표: for / foreach / LINQ / Span<T> foreach 성능 비교
// 예상: Span foreach ≈ for > foreach(array) >> LINQ(지연 평가 오버헤드)
[ShortRunJob]
[MemoryDiagnoser]
public class B1_LoopPatterns
{
    private const int N = 10_000;
    private int[] _array = null!;
    private List<int> _list = null!;

    [GlobalSetup]
    public void Setup()
    {
        _array = Enumerable.Range(0, N).ToArray();
        _list  = Enumerable.Range(0, N).ToList();
    }

    // ── 기준선: for 루프 ─────────────────────────────────────────────────
    [Benchmark(Baseline = true)]
    public long For_Array()
    {
        long sum = 0;
        for (int i = 0; i < _array.Length; i++)
            sum += _array[i];
        return sum;
    }

    // ── foreach over array ────────────────────────────────────────────────
    // 컴파일러가 배열 foreach를 for로 최적화하므로 For_Array와 거의 동일
    [Benchmark]
    public long ForEach_Array()
    {
        long sum = 0;
        foreach (int x in _array) sum += x;
        return sum;
    }

    // ── foreach over List<T> ─────────────────────────────────────────────
    // List<T>.GetEnumerator()는 구조체 → 박싱 없음, 약간의 오버헤드
    [Benchmark]
    public long ForEach_List()
    {
        long sum = 0;
        foreach (int x in _list) sum += x;
        return sum;
    }

    // ── Span<T> foreach ──────────────────────────────────────────────────
    // Span은 경계 검사가 제거되어 for와 동등하거나 더 빠름
    [Benchmark]
    public long ForEach_Span()
    {
        long sum = 0;
        foreach (int x in _array.AsSpan()) sum += x;
        return sum;
    }

    // ── ref foreach over Span<T> ─────────────────────────────────────────
    // 요소를 직접 수정할 수 있는 패턴; 읽기 전용이라도 동일 성능
    [Benchmark]
    public long RefForEach_Span()
    {
        long sum = 0;
        Span<int> span = _array;
        foreach (ref int x in span) sum += x;
        return sum;
    }

    // ── LINQ: Where + Sum ───────────────────────────────────────────────
    // 지연 평가 + 델리게이트 호출 오버헤드 → 루프보다 느림
    [Benchmark]
    public long Linq_Where_Sum()
    {
        return _array.Where(x => x % 2 == 0).Sum(x => (long)x);
    }

    // ── LINQ: Sum 단독 ──────────────────────────────────────────────────
    // Where 없이 Sum만 — 그래도 델리게이트 오버헤드 있음
    [Benchmark]
    public long Linq_Sum()
    {
        return _array.Sum(x => (long)x);
    }

    // ── Parallel.For ────────────────────────────────────────────────────
    // 작은 N에서는 병렬화 오버헤드가 이익보다 큼
    [Benchmark]
    public long Parallel_For()
    {
        long sum = 0;
        Parallel.For(0, _array.Length, () => 0L,
            (i, _, local) => local + _array[i],
            local => Interlocked.Add(ref sum, local));
        return sum;
    }
}
