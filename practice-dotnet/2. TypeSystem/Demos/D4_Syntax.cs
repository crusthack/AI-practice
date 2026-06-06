using System.Numerics;

namespace TypeSystem.Demos;

// ────────────────────────────────────────────────────────────────────────────────
// "좌표(Coordinate)" 타입을 소재로 C#의 타입 선언 문법을 전부 나열한다.
// 같은 의미의 타입을 여러 방식으로 선언할 수 있음을 보여준다.
// ────────────────────────────────────────────────────────────────────────────────
static class D4_Syntax
{
    public static void Run()
    {
        Print.Header("4. C# 타입 선언 문법 총정리");

        ShowConstructorStyles();
        ShowPropertyStyles();
        ShowMethodStyles();
        ShowInterfaceStyles();
        ShowPatternMatching();
        ShowGenericConstraints();
    }

    // ── 생성자 선언 스타일 ─────────────────────────────────────────────────────
    static void ShowConstructorStyles()
    {
        Print.Section("4-A. 생성자 선언 스타일 — 같은 타입, 다른 문법");

        // 스타일 1: 전통적 생성자 (C# 1.0)
        var s1 = new Coord_Traditional(1, 2);

        // 스타일 2: Primary constructor (C# 12)
        var s2 = new Coord_PrimaryCtor(1, 2);

        // 스타일 3: Object initializer + init-only (C# 9)
        var s3 = new Coord_InitOnly { X = 1, Y = 2 };

        // 스타일 4: Positional record (C# 9)
        var s4 = new Coord_Record(1, 2);

        // 스타일 5: readonly record struct (C# 10)
        var s5 = new Coord_ReadonlyRecordStruct(1, 2);

        // 스타일 6: Static factory method
        var s6 = Coord_Factory.Create(1, 2);
        var s7 = Coord_Factory.Origin;

        Console.WriteLine($"    Traditional:           {s1}");
        Console.WriteLine($"    Primary ctor:          {s2}");
        Console.WriteLine($"    InitOnly:              {s3}");
        Console.WriteLine($"    Record class:          {s4}");
        Console.WriteLine($"    Readonly record struct:{s5}");
        Console.WriteLine($"    Factory:               {s6}  Origin={s7}");
    }

    // ── 프로퍼티 선언 스타일 ──────────────────────────────────────────────────
    static void ShowPropertyStyles()
    {
        Print.Section("4-B. 프로퍼티 선언 스타일");

        var p = new PropShowcase(3.0, 4.0);
        Console.WriteLine($"    AutoProp (get;set;): {p.AutoProp}");
        Console.WriteLine($"    AutoProp (get;):     {p.ReadOnly}");
        Console.WriteLine($"    AutoProp (init;):    {p.InitOnly}");
        Console.WriteLine($"    Expression-bodied:   {p.Computed}");
        Console.WriteLine($"    Full (backing field):{p.Validated}");

        p.AutoProp = 99;
        Console.WriteLine($"    AutoProp 변경 후:    {p.AutoProp}");

        try { p.Validated = -1; }
        catch (ArgumentException e) { Console.WriteLine($"    음수 시도 → {e.Message}"); }
    }

    // ── 메서드 선언 스타일 ────────────────────────────────────────────────────
    static void ShowMethodStyles()
    {
        Print.Section("4-C. 메서드 선언 스타일");

        var m = new MethodShowcase();
        Console.WriteLine($"    블록 바디:         {m.BlockBody(5)}");
        Console.WriteLine($"    표현식 바디:       {m.ExpressionBody(5)}");
        Console.WriteLine($"    로컬 함수:         {m.WithLocalFunction(5)}");
        var (minVal, maxVal) = m.MinMax([3, 1, 4, 1, 5, 9]);
        Console.WriteLine($"    튜플 반환:         min={minVal}, max={maxVal}");
        m.TryDivide(10, 3, out int q);
        Console.WriteLine($"    Out 파라미터:      몫={q}  (10÷3)");
        Console.WriteLine($"    Params 배열:       {m.Sum(1, 2, 3, 4, 5)}");
        Console.WriteLine($"    선택적 파라미터:   {m.Greet("Alice")}");
        Console.WriteLine($"    선택적 파라미터:   {m.Greet("Alice", "안녕")}");
        Console.WriteLine($"    확장 메서드:       {"hello world".WordCount()}");
    }

    // ── 인터페이스 선언 스타일 ────────────────────────────────────────────────
    static void ShowInterfaceStyles()
    {
        Print.Section("4-D. 인터페이스 — 명시적 구현 & 기본 구현 (C# 8+)");

        IArea shape = new CircleArea(5);
        Console.WriteLine($"    IArea.Area: {shape.Area:F2}");
        Console.WriteLine($"    IArea.Describe (default impl): {shape.Describe()}");

        // 명시적 인터페이스 구현
        IDual dual = new DualImpl(10);
        Console.WriteLine($"    IDual explicit: Int={dual.AsInt()}, Str={dual.AsString()}");
    }

    // ── 패턴 매칭 ─────────────────────────────────────────────────────────────
    static void ShowPatternMatching()
    {
        Print.Section("4-E. 패턴 매칭 — 모든 패턴 유형");

        object?[] values =
        [
            42, -5, "hi", "hello world", 3.14,
            new PersonRecord("Alice", 30),
            new RecordPoint(1, 2),
            new Circle(5),
            null,
            (int?)7,
        ];

        foreach (var v in values)
        {
            string desc = v switch
            {
                // 타입 패턴 + when 조건
                int n when n < 0            => $"음의 정수 {n}",
                int n                       => $"양의 정수 {n}",
                // 속성 패턴
                string { Length: > 5 } s    => $"긴 문자열({s.Length}자): \"{s}\"",
                string s                    => $"짧은 문자열: \"{s}\"",
                double d                    => $"실수 {d:F2}",
                // 위치 패턴 (Deconstruct)
                RecordPoint(var x, var y)   => $"Point({x}, {y})",
                // 중첩 속성 패턴
                PersonRecord { Name: "Alice", Age: var a } => $"Alice, {a}세",
                PersonRecord { Name: var n }               => $"Person({n})",
                // 타입 패턴
                Circle c                    => $"Circle(r={c.Radius})",
                // null 패턴
                null                        => "null",
                // 기타
                _                           => $"기타({v?.GetType().Name})",
            };
            Console.WriteLine($"    {(v?.ToString() ?? "null"),-22} → {desc}");
        }
    }

    // ── Generic 제약 ──────────────────────────────────────────────────────────
    static void ShowGenericConstraints()
    {
        Print.Section("4-F. Generic 제약 (where)");

        // where T : struct — 값 타입만 허용 (박싱 원천 차단)
        Console.WriteLine($"    값 타입 합산: {Sum(1, 2, 3)}");
        Console.WriteLine($"    값 타입 합산: {Sum(1.5, 2.5, 3.0)}");

        // where T : class — 참조 타입만
        PrintIfNotNull("hello");
        PrintIfNotNull<string>(null!);

        // where T : new() — 기본 생성자 보장
        var inst = CreateDefault<Coord_InitOnly>();
        Console.WriteLine($"    new() 제약: {inst}");

        // where T : IComparable<T>
        Console.WriteLine($"    Max(3,7): {Max(3, 7)}");
        Console.WriteLine($"    Max(\"abc\",\"xyz\"): {Max("abc", "xyz")}");
    }

    static T Sum<T>(params T[] values) where T : struct, System.Numerics.INumber<T>
    {
        T r = T.Zero;
        foreach (var v in values) r += v;
        return r;
    }

    static void PrintIfNotNull<T>(T? value) where T : class
    {
        if (value is not null)
            Console.WriteLine($"    not null: {value}");
        else
            Console.WriteLine($"    null 전달됨 — 무시");
    }

    static T CreateDefault<T>() where T : new() => new T();

    static T Max<T>(T a, T b) where T : IComparable<T> =>
        a.CompareTo(b) >= 0 ? a : b;
}

// ── 생성자 스타일별 타입 ───────────────────────────────────────────────────────

// 스타일 1: 전통적 생성자
class Coord_Traditional
{
    public double X { get; }
    public double Y { get; }
    public Coord_Traditional(double x, double y) { X = x; Y = y; }
    public override string ToString() => $"({X}, {Y})";
}

// 스타일 2: Primary constructor (C# 12)
class Coord_PrimaryCtor(double x, double y)
{
    public double X { get; } = x;
    public double Y { get; } = y;
    public override string ToString() => $"({X}, {Y})";
}

// 스타일 3: Object initializer + init-only
class Coord_InitOnly
{
    public double X { get; init; }
    public double Y { get; init; }
    public override string ToString() => $"({X}, {Y})";
}

// 스타일 4: Positional record class
record Coord_Record(double X, double Y);

// 스타일 5: readonly record struct
readonly record struct Coord_ReadonlyRecordStruct(double X, double Y);

// 스타일 6: Static factory
class Coord_Factory
{
    public double X { get; }
    public double Y { get; }
    private Coord_Factory(double x, double y) { X = x; Y = y; }
    public static Coord_Factory Create(double x, double y) => new(x, y);
    public static Coord_Factory Origin => new(0, 0);
    public override string ToString() => $"({X}, {Y})";
}

// ── 프로퍼티 스타일 ───────────────────────────────────────────────────────────

class PropShowcase(double x, double y)
{
    // 1. Auto property — get;set;
    public double AutoProp { get; set; } = x + y;

    // 2. Auto property — get; (생성자에서만 설정)
    public double ReadOnly { get; } = x * y;

    // 3. Auto property — init; (object initializer에서만 설정)
    public double InitOnly { get; init; } = x - y;

    // 4. Expression-bodied getter — 계산 속성
    public double Computed => Math.Sqrt(x * x + y * y);

    // 5. Full property with backing field + 유효성 검사
    private double _validated = x;
    public double Validated
    {
        get => _validated;
        set => _validated = value >= 0
            ? value
            : throw new ArgumentException("0 이상이어야 합니다");
    }
}

// ── 메서드 스타일 ─────────────────────────────────────────────────────────────

class MethodShowcase
{
    // 1. 블록 바디
    public int BlockBody(int n)
    {
        int result = 0;
        for (int i = 1; i <= n; i++) result += i;
        return result;
    }

    // 2. 표현식 바디 (expression-bodied member)
    public int ExpressionBody(int n) => n * (n + 1) / 2;

    // 3. 로컬 함수
    public int WithLocalFunction(int n)
    {
        return Recurse(n);
        int Recurse(int x) => x <= 0 ? 0 : x + Recurse(x - 1);
    }

    // 4. 튜플 반환
    public (int Min, int Max) MinMax(int[] arr) => (arr.Min(), arr.Max());

    // 5. out 파라미터
    public bool TryDivide(int a, int b, out int quotient)
    {
        if (b == 0) { quotient = 0; return false; }
        quotient = a / b;
        return true;
    }

    // 6. params — 가변 인수
    public int Sum(params int[] values) => values.Sum();

    // 7. 선택적 파라미터 + 명명된 인수
    public string Greet(string name, string greeting = "Hello") =>
        $"{greeting}, {name}!";
}

// 확장 메서드는 반드시 static class 안에
static class StringExtensions
{
    public static int WordCount(this string s) =>
        string.IsNullOrWhiteSpace(s)
            ? 0
            : s.Split(' ', StringSplitOptions.RemoveEmptyEntries).Length;
}

// ── 인터페이스 ────────────────────────────────────────────────────────────────

interface IArea
{
    double Area { get; }
    // 기본 구현 (C# 8+) — 파생 타입이 재정의하지 않으면 이 구현 사용
    string Describe() => $"넓이={Area:F2}";
}

// 명시적 인터페이스 구현 예시
interface IDual
{
    int    AsInt();
    string AsString();
}

sealed class CircleArea(double radius) : IArea
{
    public double Area => Math.PI * radius * radius;
    // Describe()는 IArea 기본 구현 그대로 사용
}

sealed class DualImpl(int value) : IDual
{
    // 명시적 구현 — IDual 변수로만 접근 가능, 인스턴스 변수로 직접 호출 불가
    int    IDual.AsInt()    => value;
    string IDual.AsString() => value.ToString("X4");
}
