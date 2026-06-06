using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using TypeSystem;

// ──────────────────────────────────────────────────────────────────────────────
// 박싱/언박싱 비용 벤치마크
//
// 박싱: 값 타입 → object (힙 할당 + 복사)
// 언박싱: object → 값 타입 (타입 검사 + 복사)
//
// 예측:
//   Generic (박싱 없음) << NonGeneric (박싱 있음)
//   IEquatable<T> (박싱 없음) < Equals(object) (박싱 있음)
// ──────────────────────────────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class BoxingCostBench
{
    [Params(10_000)]
    public int N;

    // ── 비제네릭 vs 제네릭 컬렉션 ────────────────────────────────────────────

    [Benchmark(Baseline = true)]
    public int NonGenericList()
    {
        // ArrayList: 모든 Add가 박싱, 모든 인덱스 접근이 언박싱
        var list = new System.Collections.ArrayList(N);
        for (int i = 0; i < N; i++) list.Add(i);   // 박싱
        int sum = 0;
        for (int i = 0; i < N; i++) sum += (int)list[i]!;  // 언박싱
        return sum;
    }

    [Benchmark]
    public int GenericList()
    {
        // List<int>: 박싱 없음
        var list = new List<int>(N);
        for (int i = 0; i < N; i++) list.Add(i);
        int sum = 0;
        for (int i = 0; i < N; i++) sum += list[i];
        return sum;
    }

    // ── interface를 통한 접근 ─────────────────────────────────────────────────

    [Benchmark]
    public int InterfaceBoxing()
    {
        // IComparable: struct 대입 시 박싱
        int count = 0;
        for (int i = 0; i < N; i++)
        {
            IComparable c = i;              // 박싱
            if (c.CompareTo(i - 1) > 0) count++;
        }
        return count;
    }

    [Benchmark]
    public int DirectComparison()
    {
        // 직접 비교: 박싱 없음
        int count = 0;
        for (int i = 0; i < N; i++)
        {
            if (i > i - 1) count++;
        }
        return count;
    }

    // ── Equals(object) vs IEquatable<T>.Equals ───────────────────────────────

    private ImmutablePoint[] _points = null!;

    [GlobalSetup]
    public void Setup()
    {
        _points = new ImmutablePoint[N];
        for (int i = 0; i < N; i++)
            _points[i] = new ImmutablePoint(i, i);
    }

    [Benchmark]
    public int ObjectEquals()
    {
        // Equals(object): 인수를 object로 박싱 후 비교
        var target = new ImmutablePoint(N / 2, N / 2);
        int count = 0;
        for (int i = 0; i < N; i++)
            if (_points[i].Equals((object)target)) count++;  // 강제 박싱
        return count;
    }

    [Benchmark]
    public int IEquatableEquals()
    {
        // IEquatable<T>.Equals: 박싱 없음
        var target = new ImmutablePoint(N / 2, N / 2);
        int count = 0;
        for (int i = 0; i < N; i++)
            if (_points[i].Equals(target)) count++;
        return count;
    }
}
