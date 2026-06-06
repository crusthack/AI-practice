using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using TypeSystem;

// ──────────────────────────────────────────────────────────────────────────────
// Record / class / struct 동등성 비교 벤치마크
//
// record class : 컴파일러 자동 생성 Equals (모든 프로퍼티 순차 비교)
// class        : 기본 ReferenceEquals (주소만 비교, 매우 빠름)
// record struct: 값 타입 + 자동 생성 Equals
// IEquatable<T>: 직접 구현 (박싱 없음)
//
// 예측:
//   ReferenceEquals << IEquatable<T> ≈ record struct < record class
// ──────────────────────────────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class RecordEqualityBench
{
    [Params(100_000)]
    public int N;

    private PersonClass    _classA  = null!;
    private PersonClass    _classB  = null!;
    private PersonRecord   _recA    = null!;
    private PersonRecord   _recB    = null!;
    private RecordPoint    _rpA;
    private RecordPoint    _rpB;
    private ImmutablePoint _ipA;
    private ImmutablePoint _ipB;

    [GlobalSetup]
    public void Setup()
    {
        _classA = new PersonClass("Alice", 30);
        _classB = new PersonClass("Alice", 30);
        _recA   = new PersonRecord("Alice", 30);
        _recB   = new PersonRecord("Alice", 30);
        _rpA    = new RecordPoint(1.5, 2.5);
        _rpB    = new RecordPoint(1.5, 2.5);
        _ipA    = new ImmutablePoint(1.5, 2.5);
        _ipB    = new ImmutablePoint(1.5, 2.5);
    }

    // ── 참조 동등성 (가장 빠름) ──────────────────────────────────────────────

    [Benchmark(Baseline = true)]
    public int ReferenceEquals_Class()
    {
        int count = 0;
        for (int i = 0; i < N; i++)
            if (ReferenceEquals(_classA, _classB)) count++;
        return count;
    }

    // ── record class 자동 생성 Equals ────────────────────────────────────────

    [Benchmark]
    public int RecordClass_Equals()
    {
        int count = 0;
        for (int i = 0; i < N; i++)
            if (_recA == _recB) count++;
        return count;
    }

    // ── record struct 자동 생성 Equals ───────────────────────────────────────

    [Benchmark]
    public int RecordStruct_Equals()
    {
        int count = 0;
        for (int i = 0; i < N; i++)
            if (_rpA == _rpB) count++;
        return count;
    }

    // ── IEquatable<T> 직접 구현 (박싱 없음) ─────────────────────────────────

    [Benchmark]
    public int IEquatable_Equals()
    {
        int count = 0;
        for (int i = 0; i < N; i++)
            if (_ipA.Equals(_ipB)) count++;
        return count;
    }

    // ── with 표현식 비용 (새 인스턴스 생성) ─────────────────────────────────

    [Benchmark]
    public PersonRecord RecordClass_With()
    {
        PersonRecord current = _recA;
        for (int i = 0; i < 100; i++)
            current = current with { Age = i };
        return current;
    }

    [Benchmark]
    public RecordPoint RecordStruct_With()
    {
        RecordPoint current = _rpA;
        for (int i = 0; i < 100; i++)
            current = current with { X = i };
        return current;
    }
}
