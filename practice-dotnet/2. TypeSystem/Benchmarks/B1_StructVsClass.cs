using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using TypeSystem;

// ──────────────────────────────────────────────────────────────────────────────
// struct vs class — 할당 비용 & 접근 패턴 비교
//
// struct: 스택 할당(로컬 변수) 또는 배열 내 인라인 저장 → GC 부담 없음
// class : 항상 힙 할당 → GC 압박, 캐시 미스 가능성
//
// 예측:
//   Allocation: struct << class  (힙 할당 vs 스택)
//   Array sum : struct < class   (struct 배열은 메모리 연속, class 배열은 포인터 추적)
// ──────────────────────────────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class StructVsClassBench
{
    [Params(1_000)]
    public int N;

    // ── 단일 인스턴스 할당 비용 ────────────────────────────────────────────────

    [Benchmark(Baseline = true)]
    public double StructAlloc()
    {
        double sum = 0;
        for (int i = 0; i < N; i++)
        {
            // struct: 스택 할당, GC 없음
            var p = new MutablePoint(i, i);
            sum += p.Distance;
        }
        return sum;
    }

    [Benchmark]
    public double ClassAlloc()
    {
        double sum = 0;
        for (int i = 0; i < N; i++)
        {
            // class: 힙 할당, GC 추적 대상
            var p = new PersonClass($"p{i}", i);
            sum += p.Age;
        }
        return sum;
    }

    // ── 배열 순회 — 메모리 연속성 효과 ──────────────────────────────────────

    private MutablePoint[]  _structArr = null!;
    private PersonClass[]   _classArr  = null!;

    [GlobalSetup]
    public void Setup()
    {
        _structArr = new MutablePoint[N];
        _classArr  = new PersonClass[N];
        for (int i = 0; i < N; i++)
        {
            _structArr[i] = new MutablePoint(i, i);
            _classArr[i]  = new PersonClass($"p{i}", i);
        }
    }

    [Benchmark]
    public double StructArraySum()
    {
        // struct 배열: 요소가 연속 메모리에 inline 저장 → 캐시 친화적
        double sum = 0;
        foreach (ref readonly var p in _structArr.AsSpan())
            sum += p.Distance;
        return sum;
    }

    [Benchmark]
    public long ClassArraySum()
    {
        // class 배열: 요소는 참조(포인터) — 실제 데이터는 힙에 흩어짐
        long sum = 0;
        foreach (var p in _classArr)
            sum += p.Age;
        return sum;
    }
}
