using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using TypeSystem;

// ──────────────────────────────────────────────────────────────────────────────
// 방어적 복사(Defensive Copy) 벤치마크
//
// mutable struct를 `in` 파라미터 / readonly 필드로 전달할 때
// 컴파일러는 struct가 메서드 내부에서 변경될까 봐 복사본을 만든다.
// → 방어적 복사(defensive copy)
//
// readonly struct는 컴파일러가 방어적 복사를 생성하지 않음.
//
// 예측:
//   ReadonlyStruct << MutableStruct  (in 파라미터에서)
// ──────────────────────────────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class DefensiveCopyBench
{
    [Params(100_000)]
    public int N;

    // ── in 파라미터: mutable struct → 방어적 복사 발생 ───────────────────────

    [Benchmark(Baseline = true)]
    public double MutableStructInParam()
    {
        var p = new MutablePoint(3, 4);
        double sum = 0;
        for (int i = 0; i < N; i++)
            sum += GetDistanceMutable(in p);  // 매번 방어적 복사 발생
        return sum;
    }

    [Benchmark]
    public double ReadonlyStructInParam()
    {
        var p = new ImmutablePoint(3, 4);
        double sum = 0;
        for (int i = 0; i < N; i++)
            sum += GetDistanceImmutable(in p);  // 방어적 복사 없음
        return sum;
    }

    // mutable struct: 컴파일러가 복사본을 만들어 전달
    static double GetDistanceMutable(in MutablePoint p) => p.Distance;

    // readonly struct: 복사 없이 참조로 전달
    static double GetDistanceImmutable(in ImmutablePoint p) => p.Distance;

    // ── 반복 접근: readonly field 의 struct ──────────────────────────────────

    private readonly MutablePoint  _mutable   = new(3, 4);
    private readonly ImmutablePoint _immutable = new(3, 4);

    [Benchmark]
    public double MutableReadonlyField()
    {
        // readonly 필드의 mutable struct 접근 → 매번 복사
        double sum = 0;
        for (int i = 0; i < N; i++)
            sum += _mutable.Distance;  // _mutable을 복사해서 Distance 호출
        return sum;
    }

    [Benchmark]
    public double ImmutableReadonlyField()
    {
        // readonly 필드의 readonly struct → 복사 없음
        double sum = 0;
        for (int i = 0; i < N; i++)
            sum += _immutable.Distance;
        return sum;
    }
}
