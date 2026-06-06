namespace Iteration.Demos;

// ────────────────────────────────────────────────────────────────────────────
// LINQ (Language Integrated Query)
//   - 메서드 구문:  sequence.Where(pred).Select(proj)...
//   - 쿼리 구문:    from x in seq where pred select proj
//   - 지연 평가: Where/Select 등 대부분 연산자는 열거할 때 실행
//   - 즉시 평가: ToList/ToArray/Count/First/Sum 등 소비 연산자가 실행 트리거
// ────────────────────────────────────────────────────────────────────────────
static class D5_LINQ
{
    public static void Run()
    {
        Print.Header("5. LINQ — 언어 통합 쿼리");

        var students = SampleData.Students();
        var products = SampleData.Products();

        ShowFiltering(students);
        ShowProjection(students);
        ShowOrdering(students, products);
        ShowGrouping(students, products);
        ShowJoin(students, products);
        ShowAggregation(students, products);
        ShowQuantifiers(students, products);
        ShowElementOps(students);
        ShowGeneration();
        ShowPartitioning(students);
        ShowSetOps();
        ShowConversion(students);
        ShowQuerySyntax(students);
        ShowChaining(students, products);
    }

    // ── 5-1. 필터링 (Filtering) ───────────────────────────────────────────────
    static void ShowFiltering(IReadOnlyList<Student> students)
    {
        Print.Section("5-1. 필터링 — Where, OfType");
        {
            // Where: 조건 필터
            var csStudents = students.Where(s => s.Department == "CS");
            Console.Write("    CS 학생: ");
            Print.Items(csStudents.Select(s => s.Name), "");

            // 복합 조건
            var topCS = students.Where(s => s.Department == "CS" && s.Gpa >= 3.8);
            Console.WriteLine($"    CS GPA≥3.8: {string.Join(", ", topCS.Select(s => s.Name))}");

            // OfType<T>: 특정 타입만 필터링
            object[] mixed = [1, "hello", 2.5, "world", 42, true];
            var strings = mixed.OfType<string>();
            Console.Write("    OfType<string>: ");
            Print.Items(strings, "");
        }
    }

    // ── 5-2. 투영 (Projection) ────────────────────────────────────────────────
    static void ShowProjection(IReadOnlyList<Student> students)
    {
        Print.Section("5-2. 투영 — Select, SelectMany, Zip");
        {
            // Select: 변환
            var names = students.Select(s => s.Name.ToUpper());
            Console.Write("    Select 대문자: ");
            Print.Items(names.Take(4), "");

            // Select with index
            var indexed = students.Select((s, i) => $"{i + 1}.{s.Name}");
            Console.Write("    Select+index: ");
            Print.Items(indexed.Take(4), "");

            // 익명 형식(anonymous type) / 튜플
            var summary = students
                .Where(s => s.Gpa >= 3.5)
                .Select(s => new { s.Name, s.Gpa });
            Console.Write("    익명 형식: ");
            foreach (var x in summary) Console.Write($"{x.Name}({x.Gpa:F1}) ");
            Console.WriteLine();

            // SelectMany: 중첩 컬렉션 평탄화
            var departments = new[] { "CS", "Math" };
            var words = new[] { new[] { "a", "b" }, new[] { "c", "d", "e" } };
            var flat = words.SelectMany(w => w);
            Console.Write("    SelectMany 평탄화: ");
            Print.Items(flat, "");

            // Zip: 두 시퀀스를 쌍으로
            int[] nums   = [1, 2, 3, 4];
            string[] abc = ["A", "B", "C", "D"];
            var zipped = nums.Zip(abc, (n, s) => $"{n}{s}");
            Console.Write("    Zip: ");
            Print.Items(zipped, "");
        }
    }

    // ── 5-3. 정렬 (Ordering) ─────────────────────────────────────────────────
    static void ShowOrdering(IReadOnlyList<Student> students, IReadOnlyList<Product> products)
    {
        Print.Section("5-3. 정렬 — OrderBy, ThenBy, Reverse");
        {
            // OrderBy
            var byGpa = students.OrderBy(s => s.Gpa);
            Console.Write("    GPA 오름차순: ");
            Print.Items(byGpa.Select(s => $"{s.Name}({s.Gpa:F1})"), "");

            // OrderByDescending + ThenBy (다중 정렬)
            var byDeptThenGpa = students
                .OrderBy(s => s.Department)
                .ThenByDescending(s => s.Gpa);
            Console.WriteLine("    학과 오름→GPA 내림:");
            foreach (var s in byDeptThenGpa)
                Console.Write($"      {s.Department}/{s.Name}({s.Gpa:F1}) ");
            Console.WriteLine();

            // Reverse
            var rev = new[] { 1, 2, 3, 4, 5 }.Reverse();
            Console.Write("    Reverse: ");
            Print.Items(rev, "");
        }
    }

    // ── 5-4. 그룹화 (Grouping) ───────────────────────────────────────────────
    static void ShowGrouping(IReadOnlyList<Student> students, IReadOnlyList<Product> products)
    {
        Print.Section("5-4. 그룹화 — GroupBy, ToLookup");
        {
            // GroupBy
            var byDept = students.GroupBy(s => s.Department);
            foreach (var g in byDept.OrderBy(g => g.Key))
            {
                Console.WriteLine($"    [{g.Key}] {g.Count()}명 " +
                                  $"평균GPA={g.Average(s => s.Gpa):F2}: " +
                                  $"{string.Join(",", g.Select(s => s.Name))}");
            }

            // GroupBy with result selector
            var stats = students
                .GroupBy(s => s.Department)
                .Select(g => new
                {
                    Dept    = g.Key,
                    Count   = g.Count(),
                    MaxGpa  = g.Max(s => s.Gpa),
                    AvgGpa  = g.Average(s => s.Gpa),
                });
            foreach (var st in stats.OrderBy(s => s.Dept))
                Console.WriteLine($"    {st.Dept}: 인원={st.Count} 최고={st.MaxGpa:F1} 평균={st.AvgGpa:F2}");

            // ToLookup: 미리 인덱싱한 읽기 전용 다중 맵
            var lookup = products.ToLookup(p => p.Category);
            Console.Write("    ToLookup[전자]: ");
            Print.Items(lookup["전자"].Select(p => p.Name), "");
        }
    }

    // ── 5-5. 조인 (Join) ─────────────────────────────────────────────────────
    static void ShowJoin(IReadOnlyList<Student> students, IReadOnlyList<Product> products)
    {
        Print.Section("5-5. 조인 — Join, GroupJoin");
        {
            // 샘플 데이터 (학과별 대표 제품)
            var deptProducts = new[]
            {
                (Dept: "CS",      Product: "노트북"),
                (Dept: "Math",    Product: "계산기"),
                (Dept: "CS",      Product: "마우스"),
                (Dept: "Physics", Product: "측정기"),
            };

            // Inner Join
            var joined = students.Join(
                deptProducts,
                s => s.Department,
                dp => dp.Dept,
                (s, dp) => $"{s.Name}→{dp.Product}");
            Console.Write("    Inner Join: ");
            Print.Items(joined.Take(5), "");

            // GroupJoin (Left Outer Join 효과)
            var grouped = students
                .GroupBy(s => s.Department)
                .Select(g => g.Key)
                .Distinct()
                .GroupJoin(
                    deptProducts,
                    dept => dept,
                    dp => dp.Dept,
                    (dept, dps) => $"{dept}:[{string.Join(",", dps.Select(dp => dp.Product))}]");
            Console.Write("    GroupJoin: ");
            Print.Items(grouped, "");
        }
    }

    // ── 5-6. 집계 (Aggregation) ───────────────────────────────────────────────
    static void ShowAggregation(IReadOnlyList<Student> students, IReadOnlyList<Product> products)
    {
        Print.Section("5-6. 집계 — Count, Sum, Min, Max, Average, Aggregate");
        {
            var gpas = students.Select(s => s.Gpa);

            Console.WriteLine($"    Count:   {students.Count()}");
            Console.WriteLine($"    Sum GPA: {gpas.Sum():F2}");
            Console.WriteLine($"    Min GPA: {gpas.Min():F2}");
            Console.WriteLine($"    Max GPA: {gpas.Max():F2}");
            Console.WriteLine($"    Avg GPA: {gpas.Average():F2}");

            // Aggregate: 범용 누적 연산
            var product = Enumerable.Range(1, 5).Aggregate(1, (acc, x) => acc * x);
            Console.WriteLine($"    Aggregate 5!: {product}");

            // Aggregate with seed + result selector
            var csv = new[] { "a", "b", "c" }
                .Aggregate("값: ", (acc, s) => acc + s + ",", r => r.TrimEnd(','));
            Console.WriteLine($"    Aggregate CSV: {csv}");

            // 조건부 Count
            Console.WriteLine($"    GPA≥3.5 수: {students.Count(s => s.Gpa >= 3.5)}");

            // LongCount (long 반환)
            Console.WriteLine($"    LongCount:   {students.LongCount()}L");

            // 제품 가격 통계
            Console.WriteLine($"    최고가: {products.Max(p => p.Price):C}");
            Console.WriteLine($"    최저가: {products.Min(p => p.Price):C}");
            Console.WriteLine($"    평균가: {products.Average(p => p.Price):C}");
        }
    }

    // ── 5-7. 한정자 (Quantifiers) ─────────────────────────────────────────────
    static void ShowQuantifiers(IReadOnlyList<Student> students, IReadOnlyList<Product> products)
    {
        Print.Section("5-7. 한정자 — Any, All, Contains");
        {
            Console.WriteLine($"    Any(GPA>3.8): {students.Any(s => s.Gpa > 3.8)}");
            Console.WriteLine($"    All(GPA>2.0): {students.All(s => s.Gpa > 2.0)}");
            Console.WriteLine($"    Any():        {new int[0].Any()}  (빈 시퀀스)");

            // Contains
            var names = students.Select(s => s.Name).ToList();
            Console.WriteLine($"    Contains(Alice): {names.Contains("Alice")}");
            Console.WriteLine($"    Contains(Zara):  {names.Contains("Zara")}");

            // 재고 있는 제품 존재?
            Console.WriteLine($"    재고 있는 제품: {products.Any(p => p.IsAvailable)}");
            Console.WriteLine($"    모두 재고 있음: {products.All(p => p.IsAvailable)}");
        }
    }

    // ── 5-8. 요소 조작 (Element Ops) ─────────────────────────────────────────
    static void ShowElementOps(IReadOnlyList<Student> students)
    {
        Print.Section("5-8. 요소 — First, Last, Single, ElementAt, DefaultIfEmpty");
        {
            Console.WriteLine($"    First():              {students.First().Name}");
            Console.WriteLine($"    First(GPA>3.8):       {students.First(s => s.Gpa > 3.8).Name}");
            Console.WriteLine($"    FirstOrDefault(none): {students.FirstOrDefault(s => s.Gpa > 5.0)?.Name ?? "null"}");
            Console.WriteLine($"    Last():               {students.Last().Name}");
            Console.WriteLine($"    LastOrDefault():      {students.LastOrDefault(s => s.Department == "Math")?.Name}");

            // Single — 정확히 1개일 때
            try
            {
                var alice = students.Single(s => s.Name == "Alice");
                Console.WriteLine($"    Single(Alice):        {alice.Name}");
            }
            catch (InvalidOperationException) { }

            // Single — 여러 개이면 예외
            try { students.Single(s => s.Department == "CS"); }
            catch (InvalidOperationException e) { Console.WriteLine($"    Single(CS): {e.Message[..30]}..."); }

            Console.WriteLine($"    ElementAt(2):         {students.ElementAt(2).Name}");
            Console.WriteLine($"    ElementAtOrDefault(99): {students.ElementAtOrDefault(99)?.Name ?? "null"}");

            // DefaultIfEmpty
            var empty = new List<Student>();
            var withDefault = empty.DefaultIfEmpty();
            Console.WriteLine($"    DefaultIfEmpty 빈: {withDefault.First()?.Name ?? "null"}");
        }
    }

    // ── 5-9. 생성 (Generation) ────────────────────────────────────────────────
    static void ShowGeneration()
    {
        Print.Section("5-9. 생성 — Range, Repeat, Empty");
        {
            Console.Write("    Range(1,5):     ");
            Print.Items(Enumerable.Range(1, 5), "");

            Console.Write("    Range 제곱:     ");
            Print.Items(Enumerable.Range(1, 5).Select(x => x * x), "");

            Console.Write("    Repeat(\"★\",4): ");
            Print.Items(Enumerable.Repeat("★", 4), "");

            var empty = Enumerable.Empty<int>();
            Console.WriteLine($"    Empty<int>().Count(): {empty.Count()}");
        }
    }

    // ── 5-10. 분할 (Partitioning) ─────────────────────────────────────────────
    static void ShowPartitioning(IReadOnlyList<Student> students)
    {
        Print.Section("5-10. 분할 — Take, Skip, TakeWhile, SkipWhile, Chunk");
        {
            Console.Write("    Take(3):       ");
            Print.Items(students.Take(3).Select(s => s.Name), "");

            Console.Write("    Skip(5):       ");
            Print.Items(students.Skip(5).Select(s => s.Name), "");

            Console.Write("    TakeLast(2):   ");
            Print.Items(students.TakeLast(2).Select(s => s.Name), "");

            Console.Write("    SkipLast(6):   ");
            Print.Items(students.SkipLast(6).Select(s => s.Name), "");

            // TakeWhile / SkipWhile
            int[] nums = [2, 4, 6, 7, 8, 10];
            Console.Write("    TakeWhile(짝수): ");
            Print.Items(nums.TakeWhile(n => n % 2 == 0), "");

            Console.Write("    SkipWhile(짝수): ");
            Print.Items(nums.SkipWhile(n => n % 2 == 0), "");

            // Chunk (.NET 6+): 일정 크기로 잘라서 묶음
            Console.WriteLine("    Chunk(3):");
            foreach (var chunk in students.Chunk(3))
                Console.WriteLine($"      [{string.Join(", ", chunk.Select(s => s.Name))}]");
        }
    }

    // ── 5-11. 집합 연산 (Set Ops) ─────────────────────────────────────────────
    static void ShowSetOps()
    {
        Print.Section("5-11. 집합 — Distinct, Union, Intersect, Except, ExceptBy");
        {
            int[] a = [1, 2, 3, 4, 5];
            int[] b = [3, 4, 5, 6, 7];

            Console.Write("    Distinct([1,2,1,3]): ");
            Print.Items(new[] { 1, 2, 1, 3, 2 }.Distinct(), "");

            Console.Write("    Union:     ");
            Print.Items(a.Union(b), "");

            Console.Write("    Intersect: ");
            Print.Items(a.Intersect(b), "");

            Console.Write("    Except:    ");
            Print.Items(a.Except(b), "");

            // DistinctBy (.NET 6+)
            var words = new[] { "apple", "ant", "banana", "bear", "cherry" };
            Console.Write("    DistinctBy(첫글자): ");
            Print.Items(words.DistinctBy(w => w[0]), "");

            // UnionBy, IntersectBy, ExceptBy (.NET 6+)
            var s1 = new[] { ("A", 1), ("B", 2), ("C", 3) };
            var s2 = new[] { ("B", 99), ("C", 99), ("D", 4) };
            Console.Write("    UnionBy(key):     ");
            Print.Items(s1.UnionBy(s2, t => t.Item1).Select(t => t.Item1), "");
            Console.Write("    IntersectBy(key): ");
            Print.Items(s1.IntersectBy(s2.Select(t => t.Item1), t => t.Item1).Select(t => t.Item1), "");
            Console.Write("    ExceptBy(key):    ");
            Print.Items(s1.ExceptBy(s2.Select(t => t.Item1), t => t.Item1).Select(t => t.Item1), "");
        }
    }

    // ── 5-12. 변환 (Conversion) ───────────────────────────────────────────────
    static void ShowConversion(IReadOnlyList<Student> students)
    {
        Print.Section("5-12. 변환 — ToList, ToArray, ToDictionary, ToHashSet, Cast, AsEnumerable");
        {
            var list = students.Take(3).ToList();
            var arr  = students.Take(3).ToArray();
            Console.WriteLine($"    ToList: {list.GetType().Name}[{list.Count}]");
            Console.WriteLine($"    ToArray: {arr.GetType().Name}[{arr.Length}]");

            // ToDictionary
            var dict = students.ToDictionary(s => s.Name, s => s.Gpa);
            Console.WriteLine($"    ToDictionary[Alice]: {dict["Alice"]}");

            // ToHashSet
            var depts = students.Select(s => s.Department).ToHashSet();
            Console.Write("    ToHashSet(Dept): ");
            Print.Items(depts, "");

            // ToLookup (다중 맵)
            var lookup = students.ToLookup(s => s.Department);
            Console.WriteLine($"    ToLookup[CS]: {lookup["CS"].Count()}명");

            // Cast<T>: 강제 형변환 (실패 시 InvalidCastException)
            System.Collections.ArrayList al = [1, 2, 3];
            var casted = al.Cast<int>();
            Console.Write("    Cast<int>: ");
            Print.Items(casted, "");

            // AsEnumerable / AsQueryable
            var asEnum = students.AsEnumerable(); // IList<T> → IEnumerable<T>
            Console.WriteLine($"    AsEnumerable 타입: {asEnum.GetType().Name}");
        }
    }

    // ── 5-13. 쿼리 구문 (Query Syntax) ───────────────────────────────────────
    static void ShowQuerySyntax(IReadOnlyList<Student> students)
    {
        Print.Section("5-13. 쿼리 구문 (SQL-like syntax)");
        {
            // 기본 from-where-select
            var result =
                from s in students
                where s.Gpa >= 3.5
                orderby s.Department, s.Gpa descending
                select new { s.Name, s.Department, s.Gpa };

            Console.WriteLine("    GPA≥3.5 (쿼리 구문):");
            foreach (var r in result)
                Console.WriteLine($"      {r.Department}/{r.Name}: {r.Gpa:F1}");

            // group by
            var byDept =
                from s in students
                group s by s.Department into g
                orderby g.Key
                select new { Dept = g.Key, Avg = g.Average(s => s.Gpa) };

            Console.WriteLine("    학과별 평균 GPA (group by):");
            foreach (var g in byDept)
                Console.WriteLine($"      {g.Dept}: {g.Avg:F2}");

            // let (중간 변수)
            var withGrade =
                from s in students
                let grade = s.Gpa >= 3.7 ? "A" : s.Gpa >= 3.0 ? "B" : "C"
                where grade != "C"
                select $"{s.Name}:{grade}";

            Console.Write("    let 중간 변수: ");
            Print.Items(withGrade, "");
        }
    }

    // ── 5-14. LINQ 체이닝 & 성능 팁 ─────────────────────────────────────────
    static void ShowChaining(IReadOnlyList<Student> students, IReadOnlyList<Product> products)
    {
        Print.Section("5-14. LINQ 체이닝 & 지연 평가 시점");
        {
            // 긴 체인: 필터→그룹→집계
            var report = students
                .Where(s => s.Gpa > 2.5)
                .GroupBy(s => s.Department)
                .Select(g => new
                {
                    Dept      = g.Key,
                    BestGpa   = g.Max(s => s.Gpa),
                    BestName  = g.MaxBy(s => s.Gpa)!.Name,
                    HeadCount = g.Count(),
                })
                .OrderByDescending(x => x.BestGpa);

            Console.WriteLine("    학과 최우수 학생:");
            foreach (var r in report)
                Console.WriteLine($"      {r.Dept}: {r.BestName}({r.BestGpa:F1}), {r.HeadCount}명");

            // 즉시 평가 vs 지연 평가
            var lazyQuery = students.Where(s => s.Gpa > 3.0);  // 아직 실행 안 됨
            int count = 0;
            foreach (var s in lazyQuery) count++;   // 여기서 실행
            Console.WriteLine($"    지연 평가 후 count={count}");

            // 재사용 주의: lazy는 매번 재계산됨
            // eager = lazyQuery.ToList() 로 고정해서 재사용하면 효율적
            var eagerly = lazyQuery.ToList();
            Console.WriteLine($"    ToList 후 재사용: {eagerly.Count}명");
        }
    }
}
