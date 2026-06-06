using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using Iteration;

namespace IterBench;

// ── LINQ 체이닝 & 지연 평가 벤치마크 ────────────────────────────────────
// 목표: Eager(ToList 선행) vs Lazy LINQ chain vs 수동 루프 성능 비교
// 예상:
//   수동 루프    — 가장 빠름, 할당 없음
//   Lazy LINQ   — 지연 평가, 델리게이트 오버헤드, 할당 적음
//   Eager chain — 중간 List 할당 → GC 부담
[ShortRunJob]
[MemoryDiagnoser]
public class B3_LinqChain
{
    private const int N = 5_000;
    private Student[] _students = null!;

    [GlobalSetup]
    public void Setup()
    {
        var rng = new Random(42);
        _students = Enumerable.Range(0, N).Select(i => new Student(
            Name:       $"S{i:D5}",
            Grade:      rng.Next(1, 5),
            Gpa:        Math.Round(rng.NextDouble() * 2 + 2, 2),
            Department: (i % 4) switch { 0 => "CS", 1 => "Math", 2 => "Physics", _ => "Biology" }
        )).ToArray();
    }

    // ── Baseline: 수동 루프 ─────────────────────────────────────────────
    [Benchmark(Baseline = true)]
    public double ManualLoop()
    {
        double sum   = 0;
        int    count = 0;
        foreach (var s in _students)
        {
            if (s.Department == "CS" && s.Gpa > 3.0)
            {
                sum += s.Gpa;
                count++;
            }
        }
        return count > 0 ? sum / count : 0;
    }

    // ── Lazy LINQ: Where + Average ──────────────────────────────────────
    [Benchmark]
    public double Lazy_LINQ()
    {
        return _students
            .Where(s => s.Department == "CS" && s.Gpa > 3.0)
            .Average(s => s.Gpa);
    }

    // ── Eager: ToList 선행 후 LINQ ──────────────────────────────────────
    // ToList()로 중간 컬렉션 생성 → 추가 메모리 할당
    [Benchmark]
    public double Eager_ToList_Then_Average()
    {
        var csStudents = _students
            .Where(s => s.Department == "CS" && s.Gpa > 3.0)
            .ToList();  // 즉시 평가 → 중간 List 할당

        return csStudents.Average(s => s.Gpa);
    }

    // ── LINQ 체인: GroupBy + Select + OrderBy ──────────────────────────
    [Benchmark]
    public List<(string Dept, double AvgGpa)> Linq_GroupBy_Chain()
    {
        return _students
            .GroupBy(s => s.Department)
            .Select(g => (g.Key, g.Average(s => s.Gpa)))
            .OrderByDescending(t => t.Item2)
            .ToList();
    }

    // ── 같은 결과를 수동으로 ─────────────────────────────────────────────
    [Benchmark]
    public List<(string Dept, double AvgGpa)> Manual_GroupBy()
    {
        var map = new Dictionary<string, (double Sum, int Count)>();
        foreach (var s in _students)
        {
            var cur = map.GetValueOrDefault(s.Department);
            map[s.Department] = (cur.Sum + s.Gpa, cur.Count + 1);
        }
        return map
            .Select(kv => (kv.Key, kv.Value.Sum / kv.Value.Count))
            .OrderByDescending(t => t.Item2)
            .ToList();
    }

    // ── Lazy vs Eager: Count ────────────────────────────────────────────
    [Benchmark]
    public int Lazy_Count()
    {
        return _students.Where(s => s.Gpa > 3.5).Count();
    }

    [Benchmark]
    public int Eager_Count()
    {
        return _students.Where(s => s.Gpa > 3.5).ToList().Count;
    }

    // ── 다중 소비: Lazy(재계산) vs Eager(재사용) ──────────────────────
    [Benchmark]
    public long Lazy_MultipleEnumeration()
    {
        var query = _students.Where(s => s.Gpa > 3.0);
        // 두 번 열거 → 두 번 계산
        long c1 = query.Count();
        long c2 = query.LongCount();
        return c1 + c2;
    }

    [Benchmark]
    public long Eager_MultipleEnumeration()
    {
        var cached = _students.Where(s => s.Gpa > 3.0).ToList();  // 한 번 계산
        long c1 = cached.Count;
        long c2 = cached.LongCount();
        return c1 + c2;
    }
}
