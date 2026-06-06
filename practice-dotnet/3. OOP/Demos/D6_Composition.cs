namespace OOP.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 합성 & 패턴 (Composition & Patterns)
//   확장 메서드 · Decorator · Strategy · IDisposable 전체 패턴
// ────────────────────────────────────────────────────────────────────────────
static class D6_Composition
{
    public static void Run()
    {
        Print.Header("6. 합성 & 디자인 패턴");

        ShowExtensionMethods();
        ShowDecorator();
        ShowStrategy();
        ShowDisposable();
    }

    // ── 6-1. 확장 메서드 ────────────────────────────────────────────────────
    static void ShowExtensionMethods()
    {
        Print.Section("6-1. 확장 메서드 — string·IEnumerable·제네릭");

        // string 확장
        Print.Line($"\"  hello  \".TrimAndTitle(): {"  hello world  ".TrimAndTitle()}");
        Print.Line($"\"password123\".IsStrongPassword(): {"password123".IsStrongPassword()}");
        Print.Line($"\"P@ssw0rd!\".IsStrongPassword():   {"P@ssw0rd!".IsStrongPassword()}");
        Print.Line($"\"hello\".Repeat(3): {"hello".Repeat(3)}");

        // IEnumerable<T> 확장
        int[] nums = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        Print.Line($"nums.Median(): {nums.Median():F1}");
        Print.Line($"nums.Batch(3): {string.Join(" | ", nums.Batch(3).Select(b => $"[{string.Join(",", b)}]"))}");
        Print.Line($"nums.ForEach print first 3:");
        nums.Take(3).ForEach(n => Console.Write($"    →{n}  "));
        Console.WriteLine();

        // 제네릭 확장
        var list = new List<string> { "a", "b", "c" };
        list.AddRange2("d", "e");
        Print.Line($"AddRange2: [{string.Join(",", list)}]");

        var maybeNull = (string?)null;
        Print.Line($"null.IsNullOrEmpty(): {maybeNull.IsNullOrEmpty()}");
    }

    // ── 6-2. Decorator 패턴 ─────────────────────────────────────────────────
    static void ShowDecorator()
    {
        Print.Section("6-2. Decorator — 기능을 감싸서 확장");

        // 기본 커피
        ICoffee coffee = new SimpleCoffee();
        Print.Line($"기본: {coffee.Description,-30} {coffee.Cost():C}");

        // 데코레이터 중첩
        coffee = new MilkDecorator(coffee);
        Print.Line($"우유: {coffee.Description,-30} {coffee.Cost():C}");

        coffee = new SugarDecorator(coffee);
        Print.Line($"+설탕: {coffee.Description,-30} {coffee.Cost():C}");

        coffee = new WhipDecorator(new MilkDecorator(new SimpleCoffee()));
        Print.Line($"별도 조합: {coffee.Description,-30} {coffee.Cost():C}");
    }

    // ── 6-3. Strategy 패턴 ──────────────────────────────────────────────────
    static void ShowStrategy()
    {
        Print.Section("6-3. Strategy — 알고리즘을 런타임에 교체");

        int[] data = [64, 34, 25, 12, 22, 11, 90];

        var sorter = new Sorter<int>();

        sorter.Strategy = new BubbleSortStrategy<int>();
        var b = (int[])data.Clone();
        sorter.Sort(b);
        Print.Line($"BubbleSort: [{string.Join(",", b)}]");

        sorter.Strategy = new SelectionSortStrategy<int>();
        var s = (int[])data.Clone();
        sorter.Sort(s);
        Print.Line($"SelectionSort: [{string.Join(",", s)}]");

        // 람다로 즉석 전략 (함수형 전략)
        sorter.Strategy = new LambdaSortStrategy<int>((arr) => Array.Sort(arr, (a, z) => z.CompareTo(a)));
        var r = (int[])data.Clone();
        sorter.Sort(r);
        Print.Line($"역순 Sort: [{string.Join(",", r)}]");
    }

    // ── 6-4. IDisposable 완전 패턴 ─────────────────────────────────────────
    static void ShowDisposable()
    {
        Print.Section("6-4. IDisposable — using · Dispose(bool) · Finalizer");

        // using 문: 블록 종료 시 Dispose 자동 호출
        Print.Line("using 문:");
        using (var res = new ManagedResource("DB커넥션"))
        {
            res.DoWork();
        }   // 여기서 Dispose 호출

        // using 선언 (C# 8+): 스코프 끝에 Dispose
        Print.Line("using 선언 (C# 8+):");
        {
            using var res2 = new ManagedResource("파일핸들");
            res2.DoWork();
        }   // 여기서 Dispose 호출

        // 파이널라이저 우회 (정상 경로에선 GC.SuppressFinalize 호출됨)
        Print.Line("중첩 using:");
        using var outer = new ManagedResource("외부");
        using var inner = new ManagedResource("내부");
        inner.DoWork();
        outer.DoWork();
        // inner → outer 순으로 Dispose (역순)
    }
}

// ── 확장 메서드 ───────────────────────────────────────────────────────────

public static class StringExtensions
{
    public static string TrimAndTitle(this string s) =>
        System.Globalization.CultureInfo.CurrentCulture.TextInfo.ToTitleCase(s.Trim().ToLower());

    public static bool IsStrongPassword(this string s) =>
        s.Length >= 8 && s.Any(char.IsUpper) && s.Any(char.IsDigit) && s.Any(c => !char.IsLetterOrDigit(c));

    public static string Repeat(this string s, int times) =>
        string.Concat(Enumerable.Repeat(s, times));

    public static bool IsNullOrEmpty(this string? s) => string.IsNullOrEmpty(s);
}

public static class EnumerableExtensions
{
    // 중앙값
    public static double Median<T>(this IEnumerable<T> source) where T : IComparable<T>
    {
        var sorted = source.OrderBy(x => x).ToList();
        int mid = sorted.Count / 2;
        return sorted.Count % 2 == 1
            ? Convert.ToDouble(sorted[mid])
            : (Convert.ToDouble(sorted[mid - 1]) + Convert.ToDouble(sorted[mid])) / 2;
    }

    // 일괄 묶음 (batching)
    public static IEnumerable<IEnumerable<T>> Batch<T>(this IEnumerable<T> source, int size)
    {
        var batch = new List<T>(size);
        foreach (var item in source)
        {
            batch.Add(item);
            if (batch.Count == size) { yield return batch.ToList(); batch.Clear(); }
        }
        if (batch.Count > 0) yield return batch;
    }

    // 반환값 없는 ForEach
    public static void ForEach<T>(this IEnumerable<T> source, Action<T> action)
    {
        foreach (var item in source) action(item);
    }

    // 가변 인수 AddRange
    public static void AddRange2<T>(this IList<T> list, params T[] items)
    {
        foreach (var item in items) list.Add(item);
    }
}

// ── Decorator 패턴 ────────────────────────────────────────────────────────

public interface ICoffee
{
    string  Description { get; }
    decimal Cost();
}

public class SimpleCoffee : ICoffee
{
    public string  Description => "에스프레소";
    public decimal Cost()      => 1_500m;
}

// 모든 데코레이터의 기반 (추상 데코레이터)
public abstract class CoffeeDecorator(ICoffee inner) : ICoffee
{
    protected readonly ICoffee _inner = inner;
    public virtual string  Description => _inner.Description;
    public virtual decimal Cost()      => _inner.Cost();
}

public class MilkDecorator(ICoffee inner) : CoffeeDecorator(inner)
{
    public override string  Description => _inner.Description + " + 우유";
    public override decimal Cost()      => _inner.Cost() + 500m;
}

public class SugarDecorator(ICoffee inner) : CoffeeDecorator(inner)
{
    public override string  Description => _inner.Description + " + 설탕";
    public override decimal Cost()      => _inner.Cost() + 200m;
}

public class WhipDecorator(ICoffee inner) : CoffeeDecorator(inner)
{
    public override string  Description => _inner.Description + " + 휘핑크림";
    public override decimal Cost()      => _inner.Cost() + 800m;
}

// ── Strategy 패턴 ─────────────────────────────────────────────────────────

public interface ISortStrategy<T> where T : IComparable<T>
{
    void Sort(T[] arr);
}

public class Sorter<T> where T : IComparable<T>
{
    public ISortStrategy<T> Strategy { get; set; } = new BubbleSortStrategy<T>();
    public void Sort(T[] arr) => Strategy.Sort(arr);
}

public class BubbleSortStrategy<T> : ISortStrategy<T> where T : IComparable<T>
{
    public void Sort(T[] arr)
    {
        for (int i = 0; i < arr.Length - 1; i++)
            for (int j = 0; j < arr.Length - i - 1; j++)
                if (arr[j].CompareTo(arr[j + 1]) > 0)
                    (arr[j], arr[j + 1]) = (arr[j + 1], arr[j]);
    }
}

public class SelectionSortStrategy<T> : ISortStrategy<T> where T : IComparable<T>
{
    public void Sort(T[] arr)
    {
        for (int i = 0; i < arr.Length - 1; i++)
        {
            int min = i;
            for (int j = i + 1; j < arr.Length; j++)
                if (arr[j].CompareTo(arr[min]) < 0) min = j;
            (arr[i], arr[min]) = (arr[min], arr[i]);
        }
    }
}

// 함수형 전략: 람다로 즉석 생성
public class LambdaSortStrategy<T>(Action<T[]> sortAction) : ISortStrategy<T> where T : IComparable<T>
{
    public void Sort(T[] arr) => sortAction(arr);
}

// ── IDisposable 완전 패턴 ─────────────────────────────────────────────────

public class ManagedResource : IDisposable
{
    private readonly string _name;
    private bool _disposed;

    public ManagedResource(string name)
    {
        _name = name;
        Console.WriteLine($"    [{_name}] 열림");
    }

    public void DoWork()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        Console.WriteLine($"    [{_name}] 작업 중");
    }

    // 공개 Dispose: using / 명시적 호출
    public void Dispose()
    {
        Dispose(disposing: true);
        GC.SuppressFinalize(this);  // 파이널라이저 대기열에서 제거
    }

    // protected virtual: 파생 클래스가 추가 리소스 해제 시 override
    protected virtual void Dispose(bool disposing)
    {
        if (_disposed) return;

        if (disposing)
        {
            // 관리 리소스 해제 (다른 IDisposable 객체 등)
            Console.WriteLine($"    [{_name}] 관리 리소스 해제");
        }
        // 비관리 리소스 해제 (파일 핸들, 네이티브 메모리 등)
        // Marshal.FreeHGlobal(...)
        Console.WriteLine($"    [{_name}] 닫힘");
        _disposed = true;
    }

    // 파이널라이저: Dispose 호출 안 됐을 때 GC가 실행 (비관리 리소스 안전망)
    ~ManagedResource() => Dispose(disposing: false);
}
