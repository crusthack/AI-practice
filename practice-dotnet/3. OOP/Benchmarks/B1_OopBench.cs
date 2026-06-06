using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using OOP.Demos;
using System.Numerics;

namespace OopBench;

// ── OOP 핵심 성능 포인트 ─────────────────────────────────────────────────
// 1. 가상 디스패치: 가상 vs 비가상 호출 비용
// 2. 인터페이스 디스패치: 인터페이스 vs 직접 호출 비용
// 3. 제네릭 vs 박싱: List<int> vs ArrayList (박싱 비용)
// 4. delegate vs 직접 호출: Func<> 오버헤드
// 예상: 직접호출 > 제네릭 > 가상 ≈ 인터페이스 > delegate > 박싱
[ShortRunJob]
[MemoryDiagnoser]
public class B1_OopBench
{
    private const int N = 10_000;

    private Circle[]   _circles = null!;
    private Shape[]    _shapes  = null!;   // Animal로 참조 (가상 디스패치)
    private IDrawable[] _iDrawables = null!;

    private System.Collections.ArrayList _arrayList = null!;
    private List<int> _genericList = null!;

    private Func<double, double> _funcSqrt = null!;

    [GlobalSetup]
    public void Setup()
    {
        _circles    = Enumerable.Range(1, N).Select(i => new Circle(i)).ToArray();
        _shapes     = _circles;
        _iDrawables = Enumerable.Range(1, N).Select(i => (IDrawable)new Circle2(i)).ToArray();

        _arrayList  = new System.Collections.ArrayList(Enumerable.Range(0, N).Cast<object>().ToArray());
        _genericList = new List<int>(Enumerable.Range(0, N));

        _funcSqrt = Math.Sqrt;
    }

    // ── 직접 호출 (기준선) ───────────────────────────────────────────────────
    [Benchmark(Baseline = true)]
    public double DirectCall()
    {
        double sum = 0;
        foreach (var c in _circles) sum += c.Area();
        return sum;
    }

    // ── 가상 디스패치 ────────────────────────────────────────────────────────
    [Benchmark]
    public double VirtualDispatch()
    {
        double sum = 0;
        foreach (Shape s in _shapes) sum += s.Area();
        return sum;
    }

    // ── 인터페이스 디스패치 ──────────────────────────────────────────────────
    [Benchmark]
    public int InterfaceDispatch()
    {
        int len = 0;
        foreach (IDrawable d in _iDrawables) len += d.Draw().Length;
        return len;
    }

    // ── 제네릭 List<int> (박싱 없음) ────────────────────────────────────────
    [Benchmark]
    public long GenericList_Sum()
    {
        long sum = 0;
        for (int i = 0; i < _genericList.Count; i++) sum += _genericList[i];
        return sum;
    }

    // ── ArrayList (박싱 발생) ────────────────────────────────────────────────
    [Benchmark]
    public long ArrayList_Sum()
    {
        long sum = 0;
        for (int i = 0; i < _arrayList.Count; i++) sum += (int)_arrayList[i]!;
        return sum;
    }

    // ── Func<> 델리게이트 호출 ──────────────────────────────────────────────
    [Benchmark]
    public double DelegateCall()
    {
        double sum = 0;
        for (int i = 1; i <= N; i++) sum += _funcSqrt(i);
        return sum;
    }

    // ── 직접 정적 메서드 호출 ───────────────────────────────────────────────
    [Benchmark]
    public double StaticCall()
    {
        double sum = 0;
        for (int i = 1; i <= N; i++) sum += Math.Sqrt(i);
        return sum;
    }

    // ── 제네릭 수학 vs 구체 타입 ────────────────────────────────────────────
    [Benchmark]
    public int GenericMathSum_Int() => GenericMath.Sum(Enumerable.Range(1, 100).ToArray());

    [Benchmark]
    public int DirectSum_Int()
    {
        int sum = 0;
        for (int i = 1; i <= 100; i++) sum += i;
        return sum;
    }
}
