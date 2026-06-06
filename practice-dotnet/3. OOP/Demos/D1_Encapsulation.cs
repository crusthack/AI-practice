namespace OOP.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 캡슐화 (Encapsulation)
//   접근 한정자 6종 · 프로퍼티 7종 · 필드 패턴 · 생성자 패턴
// ────────────────────────────────────────────────────────────────────────────
static class D1_Encapsulation
{
    public static void Run()
    {
        Print.Header("1. 캡슐화 (Encapsulation)");

        ShowAccessModifiers();
        ShowProperties();
        ShowFields();
        ShowConstructors();
    }

    // ── 1-1. 접근 한정자 ─────────────────────────────────────────────────────
    static void ShowAccessModifiers()
    {
        Print.Section("1-1. 접근 한정자 6종");

        var obj = new AccessDemo();
        Print.Line($"public 접근:             {obj.PublicProp}");
        Print.Line($"internal 접근(같은 어셈블리): {obj.InternalProp}");
        // protected, private, protected internal, private protected:
        // → 외부에서 직접 접근 불가; 파생 클래스 / 어셈블리 내부에서만 가능
        Print.Line($"private   (리플렉션 확인): {typeof(AccessDemo).GetField("_private", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance)?.Name}");
        Print.Line($"protected (파생 클래스에서만 접근 가능)");
        Print.Line($"protected internal = same assembly OR derived");
        Print.Line($"private protected  = same assembly AND derived");

        var derived = new AccessDerived();
        Print.Line($"파생 클래스에서 protected 접근: {derived.ReadProtected()}");
    }

    // ── 1-2. 프로퍼티 7종 ────────────────────────────────────────────────────
    static void ShowProperties()
    {
        Print.Section("1-2. 프로퍼티 7종");

        // 생성 (required + init)
        var p = new PropShowcase
        {
            RequiredId = "ID-001",
            FullName   = "홍길동",
        };

        Print.Line($"auto get;set;        → {p.AutoProp}");
        Print.Line($"auto get; (readonly) → {p.ReadOnly}");
        Print.Line($"init-only            → {p.InitOnly}");
        Print.Line($"required             → {p.RequiredId}");
        Print.Line($"expression-bodied    → {p.FullName}");
        Print.Line($"계산 프로퍼티        → {p.WordCount}단어");
        p.AutoProp = "변경됨";
        Print.Line($"변경 후 AutoProp     → {p.AutoProp}");

        // full backing field
        p.Clamped = 150;   // 내부에서 0~100으로 클램핑
        Print.Line($"full backing field (클램핑 150→100): {p.Clamped}");
    }

    // ── 1-3. 필드 패턴 ───────────────────────────────────────────────────────
    static void ShowFields()
    {
        Print.Section("1-3. 필드 패턴 — const / readonly / static");

        Print.Line($"const PI:        {FieldDemo.PI}");
        Print.Line($"static Count:    {FieldDemo.Count}");
        var f1 = new FieldDemo("A");
        var f2 = new FieldDemo("B");
        Print.Line($"static Count(2): {FieldDemo.Count}");
        Print.Line($"readonly Name:   {f1.Name}");
        Print.Line($"ReadonlyStruct:  {FieldDemo.Origin}");

        // const vs readonly
        // const   : 컴파일 타임 상수, 값 형식/string만 가능
        // readonly: 런타임 초기화 가능, 참조 형식도 가능
    }

    // ── 1-4. 생성자 패턴 ─────────────────────────────────────────────────────
    static void ShowConstructors()
    {
        Print.Section("1-4. 생성자 패턴 — 기본/오버로드/위임/정적");

        var a = new CtorDemo();
        var b = new CtorDemo("홍");
        var c = new CtorDemo("홍", 30);
        Print.Line($"기본: {a}");
        Print.Line($"이름: {b}");
        Print.Line($"이름+나이: {c}");
        Print.Line($"생성 횟수(static): {CtorDemo.TotalCreated}");

        // Primary constructor (C# 12)
        var pc = new PrimaryCtorDemo("Alice", 25);
        Print.Line($"Primary ctor: {pc.Greet()}");
    }
}

// ── 접근 한정자 데모 클래스 ────────────────────────────────────────────────

public class AccessDemo
{
    public    string PublicProp    { get; } = "public";
    internal  string InternalProp  { get; } = "internal";
    protected string ProtectedProp { get; } = "protected";
    private   string PrivateProp   { get; } = "private";

#pragma warning disable CS0414
    internal  string _internal  = "internal_field";
    private   string _private   = "private_field";
#pragma warning restore CS0414

    protected internal string ProtectedInternalProp { get; } = "protected internal";
    private protected  string PrivateProtectedProp  { get; } = "private protected";
}

public class AccessDerived : AccessDemo
{
    // 파생 클래스에서 protected 접근
    public string ReadProtected() => ProtectedProp;
}

// ── 프로퍼티 데모 ──────────────────────────────────────────────────────────

public class PropShowcase
{
    // 1. auto get;set;
    public string AutoProp { get; set; } = "초기값";

    // 2. get; 전용 (생성자에서만 설정)
    public string ReadOnly { get; } = "읽기전용";

    // 3. init-only (객체 초기화식에서만 설정)
    public string InitOnly { get; init; } = "기본";

    // 4. required (객체 초기화식에서 반드시 설정)
    public required string RequiredId { get; init; }

    // 5. expression-bodied (get만; 계산값)
    private string _fullName = "";
    public string FullName
    {
        get => _fullName;
        init => _fullName = value ?? throw new ArgumentNullException(nameof(FullName));
    }

    // 6. 계산 프로퍼티 (backing field 없음)
    public int WordCount => FullName.Split(' ', StringSplitOptions.RemoveEmptyEntries).Length;

    // 7. full backing field with validation
    private int _clamped;
    public int Clamped
    {
        get => _clamped;
        set => _clamped = Math.Clamp(value, 0, 100);
    }
}

// ── 필드 데모 ──────────────────────────────────────────────────────────────

public class FieldDemo
{
    // const: 컴파일 타임 상수
    public const double PI = 3.14159265358979;

    // static: 인스턴스 공유
    public static int Count { get; private set; }

    // static readonly struct: 불변 공유 값
    public static readonly (int X, int Y) Origin = (0, 0);

    // readonly: 생성자 이후 불변
    public readonly string Name;

    public FieldDemo(string name)
    {
        Name = name;
        Count++;
    }
}

// ── 생성자 패턴 데모 ───────────────────────────────────────────────────────

public class CtorDemo
{
    public string Name { get; }
    public int    Age  { get; }
    public static int TotalCreated { get; private set; }

    // 정적 생성자: 클래스 첫 사용 시 딱 한 번 실행
    static CtorDemo() => TotalCreated = 0;

    // 기본 생성자
    public CtorDemo() : this("익명", 0) { }

    // 이름만
    public CtorDemo(string name) : this(name, 0) { }

    // 위임 생성자 (this(...))
    public CtorDemo(string name, int age)
    {
        Name = name;
        Age  = age;
        TotalCreated++;
    }

    public override string ToString() => $"{Name}({Age})";
}

// Primary constructor (C# 12): 파라미터가 멤버로 캡처됨
public class PrimaryCtorDemo(string name, int age)
{
    public string Greet() => $"안녕, {name}! 나이: {age}";
}
