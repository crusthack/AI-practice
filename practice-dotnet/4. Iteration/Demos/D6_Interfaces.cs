using System.Collections;

namespace Iteration.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 컬렉션 관련 핵심 인터페이스 계층
//
//   IEnumerable<T>         → foreach, LINQ의 기반
//     ICollection<T>       → Count, Add, Remove, Contains
//       IList<T>           → 인덱서, Insert, RemoveAt
//         IReadOnlyList<T> → 읽기 전용 인덱서
//
//   IComparer<T>           → 외부 비교기 (Sort, SortedDictionary 등에 주입)
//   IEqualityComparer<T>   → 외부 동등성 비교기 (Dictionary, HashSet 등에 주입)
//   IComparable<T>         → 타입 자체가 자연 정렬 순서를 정의
// ────────────────────────────────────────────────────────────────────────────
static class D6_Interfaces
{
    public static void Run()
    {
        Print.Header("6. 컬렉션 인터페이스 & 비교기");

        ShowInterfaceHierarchy();
        ShowCustomEnumerable();
        ShowComparerUsage();
        ShowEqualityComparerUsage();
        ShowIComparableUsage();
        ShowReadOnlyInterfaces();
        ShowInterfacePolymorphism();
    }

    // ── 6-1. 인터페이스 계층 시각화 ──────────────────────────────────────────
    static void ShowInterfaceHierarchy()
    {
        Print.Section("6-1. 컬렉션 인터페이스 계층 확인");
        {
            var list = new List<int> { 1, 2, 3 };

            // List<T>가 구현하는 인터페이스들
            bool isEnumerable   = list is IEnumerable<int>;
            bool isCollection   = list is ICollection<int>;
            bool isList         = list is IList<int>;
            bool isReadOnly     = list is IReadOnlyList<int>;

            Console.WriteLine($"    List<T> is IEnumerable<T>:      {isEnumerable}");
            Console.WriteLine($"    List<T> is ICollection<T>:      {isCollection}");
            Console.WriteLine($"    List<T> is IList<T>:            {isList}");
            Console.WriteLine($"    List<T> is IReadOnlyList<T>:    {isReadOnly}");

            // 타입 명시 없이 인터페이스로 받아서 사용
            ICollection<int> col = list;
            col.Add(4);
            Console.WriteLine($"    ICollection.Add(4) → Count={col.Count}");

            IList<int> ilist = list;
            ilist.Insert(0, 0);
            Console.WriteLine($"    IList.Insert(0,0) → [0]={ilist[0]}");
        }
    }

    // ── 6-2. 커스텀 IEnumerable<T> + IEnumerator<T> 재확인 ──────────────────
    static void ShowCustomEnumerable()
    {
        Print.Section("6-2. 커스텀 IEnumerable — InfiniteCounter");
        {
            // InfiniteCounter는 IEnumerable<T>를 직접 구현
            var counter = new InfiniteCounter(start: 10, step: 5);

            // Take()로 잘라서 소비
            Console.Write("    Take(5): ");
            Print.Items(counter.Take(5), "");

            // Where + Take 조합
            Console.Write("    3의 배수 5개: ");
            Print.Items(counter.Where(n => n % 3 == 0).Take(5), "");

            // IEnumerable<T>로 매개변수 전달
            Console.WriteLine($"    Sum(10개): {counter.Take(10).Sum()}");
        }
    }

    // ── 6-3. IComparer<T> — 외부 비교기 주입 ────────────────────────────────
    static void ShowComparerUsage()
    {
        Print.Section("6-3. IComparer<T> — 외부 비교기 주입");
        {
            var students = SampleData.Students().ToList();

            // 1) Comparer<T>.Create: 람다로 즉석 생성
            var byNameDesc = Comparer<Student>.Create((a, b) => string.Compare(b.Name, a.Name, StringComparison.Ordinal));
            students.Sort(byNameDesc);
            Console.Write("    이름 역순: ");
            Print.Items(students.Take(4).Select(s => s.Name), "");

            // 2) 별도 클래스로 IComparer<T> 구현
            students.Sort(new GpaDescThenNameComparer());
            Console.WriteLine("    GPA내림→이름오름:");
            foreach (var s in students.Take(4))
                Console.Write($"      {s.Name}({s.Gpa:F1})");
            Console.WriteLine();

            // 3) SortedSet에 comparer 주입
            var ss = new SortedSet<Student>(students, new GpaDescThenNameComparer());
            Console.Write("    SortedSet Min: ");
            Console.WriteLine(ss.Min?.Name);

            // 4) SortedDictionary에 comparer 주입 (키 정렬 방향 제어)
            var revDict = new SortedDictionary<string, int>(
                Comparer<string>.Create((a, b) => string.Compare(b, a, StringComparison.Ordinal)));
            revDict["c"] = 3; revDict["a"] = 1; revDict["b"] = 2;
            Console.Write("    SortedDictionary 역순 키: ");
            Print.Items(revDict.Keys, "");
        }
    }

    // ── 6-4. IEqualityComparer<T> — 동등성 비교기 ──────────────────────────
    static void ShowEqualityComparerUsage()
    {
        Print.Section("6-4. IEqualityComparer<T> — 동등성 비교기");
        {
            var students = SampleData.Students().ToList();

            // 1) Dictionary 키 비교기 (대소문자 무시)
            var dict = new Dictionary<string, int>(StringComparer.OrdinalIgnoreCase)
            {
                ["Alice"] = 100,
                ["BOB"]   = 200,
            };
            Console.WriteLine($"    dict[\"alice\"]={dict["alice"]}  dict[\"bob\"]={dict["bob"]}");

            // 2) Distinct / GroupBy에 comparer 주입
            var names = new[] { "Alice", "alice", "BOB", "bob", "Carol" };
            Console.Write("    Distinct(OrdinalIgnoreCase): ");
            Print.Items(names.Distinct(StringComparer.OrdinalIgnoreCase), "");

            // 3) 커스텀 IEqualityComparer<T>
            var sameGpa = students.Distinct(new StudentByGpaComparer()).ToList();
            Console.Write("    Distinct by GPA: ");
            Print.Items(sameGpa.Select(s => $"{s.Name}({s.Gpa:F1})"), "");

            // 4) HashSet에 comparer 주입
            var hs = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            hs.Add("Hello"); hs.Add("HELLO"); hs.Add("world");
            Console.Write("    HashSet(OrdinalIgnoreCase): ");
            Print.Items(hs, "");
        }
    }

    // ── 6-5. IComparable<T> — 타입 자체의 자연 정렬 ────────────────────────
    static void ShowIComparableUsage()
    {
        Print.Section("6-5. IComparable<T> — 타입 자체에 자연 정렬 정의");
        {
            var students = SampleData.Students().ToList();

            // Student가 IComparable<Student>를 구현함 (GPA 기준)
            students.Sort();   // IComparable<T> 사용
            Console.Write("    Sort() GPA 오름차순: ");
            Print.Items(students.Select(s => $"{s.Name}({s.Gpa:F1})"), "");

            // Min / Max — IComparable<T> 이용
            Console.WriteLine($"    Min (최저GPA): {students.Min()?.Name}");
            Console.WriteLine($"    Max (최고GPA): {students.Max()?.Name}");

            // 버전 비교 (Version은 IComparable<Version>)
            var versions = new[] { new Version(2,0), new Version(1,5), new Version(2,1), new Version(1,0) };
            Array.Sort(versions);
            Console.Write("    Version Sort: ");
            Print.Items(versions, "");

            // Temperature 레코드 (IComparable<T> 구현)
            var temps = new[] { new Temperature(37.5), new Temperature(36.5), new Temperature(38.1) };
            Array.Sort(temps);
            Console.Write("    Temperature Sort: ");
            Print.Items(temps, "");
        }
    }

    // ── 6-6. 읽기 전용 인터페이스 ───────────────────────────────────────────
    static void ShowReadOnlyInterfaces()
    {
        Print.Section("6-6. 읽기 전용 인터페이스 — IReadOnlyList, IReadOnlyCollection, IReadOnlyDictionary");
        {
            var list = new List<int> { 10, 20, 30, 40, 50 };
            var dict = new Dictionary<string, int> { ["a"] = 1, ["b"] = 2 };

            // 읽기 전용 뷰로 노출 (내부 변경 방지)
            IReadOnlyList<int>              roList = list;
            IReadOnlyCollection<int>        roCol  = list;
            IReadOnlyDictionary<string,int> roDict = dict;

            Console.WriteLine($"    IReadOnlyList[2]:     {roList[2]}");
            Console.WriteLine($"    IReadOnlyCollection.Count: {roCol.Count}");
            Console.WriteLine($"    IReadOnlyDictionary[a]:    {roDict["a"]}");

            // AsReadOnly(): List를 ReadOnlyCollection<T>으로 래핑
            var ro = list.AsReadOnly();
            Console.WriteLine($"    AsReadOnly 타입: {ro.GetType().Name}");

            // Array는 IReadOnlyList<T>를 구현함
            int[] arr = [1, 2, 3];
            IReadOnlyList<int> roArr = arr;
            Console.WriteLine($"    Array as IReadOnlyList: [{roArr[0]},{roArr[1]},{roArr[2]}]");
        }
    }

    // ── 6-7. 인터페이스 다형성 ───────────────────────────────────────────────
    static void ShowInterfacePolymorphism()
    {
        Print.Section("6-7. 인터페이스 다형성 — 여러 타입을 동일 인터페이스로 처리");
        {
            // IEnumerable<int>로 다양한 컬렉션 받기
            var sources = new IEnumerable<int>[]
            {
                new[] { 1, 2, 3 },
                new List<int> { 4, 5, 6 },
                Enumerable.Range(7, 3),
                new NumberRange(10, 12),   // 커스텀 구현체
            };

            foreach (var src in sources)
            {
                int sum = src.Sum();
                Console.WriteLine($"    {src.GetType().Name,-20} Sum={sum}");
            }

            // ICollection<T>로 다양한 컬렉션 조작
            var collections = new ICollection<string>[]
            {
                new List<string>   { "list" },
                new HashSet<string> { "set" },
            };
            foreach (var col in collections)
            {
                col.Add("added");
                Console.WriteLine($"    {col.GetType().Name,-20} Count={col.Count}  Contains(\"added\")={col.Contains("added")}");
            }
        }
    }
}

// ── 보조 타입들 ────────────────────────────────────────────────────────────

// 무한 카운터: IEnumerable<int>를 yield로 구현
public sealed class InfiniteCounter : IEnumerable<int>
{
    private readonly int _start, _step;
    public InfiniteCounter(int start, int step) { _start = start; _step = step; }

    public IEnumerator<int> GetEnumerator()
    {
        int current = _start;
        while (true)
        {
            yield return current;
            current += _step;
        }
    }

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
}

// GPA 내림차순, 이름 오름차순 비교기
sealed class GpaDescThenNameComparer : IComparer<Student>
{
    public int Compare(Student? x, Student? y)
    {
        if (x is null && y is null) return 0;
        if (x is null) return 1;
        if (y is null) return -1;

        int gpaCmp = y.Gpa.CompareTo(x.Gpa);  // 내림차순
        return gpaCmp != 0 ? gpaCmp : string.Compare(x.Name, y.Name, StringComparison.Ordinal);
    }
}

// GPA로 동등성 판단하는 비교기
sealed class StudentByGpaComparer : IEqualityComparer<Student>
{
    public bool Equals(Student? x, Student? y) =>
        x is null ? y is null : (y is not null && x.Gpa == y.Gpa);

    public int GetHashCode(Student s) => s.Gpa.GetHashCode();
}

// IComparable<T> 구현 예시
public sealed record Temperature(double Celsius) : IComparable<Temperature>
{
    public int CompareTo(Temperature? other) =>
        other is null ? 1 : Celsius.CompareTo(other.Celsius);

    public override string ToString() => $"{Celsius:F1}°C";
}
