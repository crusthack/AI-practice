namespace OOP.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 다형성 (Polymorphism)
//   인터페이스(명시/묵시 구현, default 메서드, static abstract)
//   연산자 오버로딩 · 변환 연산자 · 인덱서 · 타입 캐스팅
// ────────────────────────────────────────────────────────────────────────────
static class D3_Polymorphism
{
    public static void Run()
    {
        Print.Header("3. 다형성 (Polymorphism)");

        ShowInterfaces();
        ShowOperatorOverloading();
        ShowConversionOperators();
        ShowIndexer();
        ShowTypeCasting();
    }

    // ── 3-1. 인터페이스 ─────────────────────────────────────────────────────
    static void ShowInterfaces()
    {
        Print.Section("3-1. 인터페이스 — 묵시/명시 구현·default·static abstract");

        // 묵시적 구현
        IDrawable[] drawables = [new Circle2(3), new Square(4)];
        foreach (var d in drawables)
            Print.Line($"{d.GetType().Name,-8}: Draw={d.Draw(),-20}  Info={d.Info()}");  // default 메서드

        // 명시적 구현: ILoggable을 통해서만 접근 가능
        var sq = new Square(4);
        ILoggable log = sq;
        Print.Line($"명시적 ILoggable.Log: {log.Log()}");
        // sq.Log() → 컴파일 오류: Square에 직접 Log() 없음

        // 인터페이스 분리 원칙 (ISP)
        IResizable r = new Square(4);
        r.Resize(2.0);
        Print.Line($"Resize(×2): {((Square)r).Side}");

        // static abstract (C# 11): 인터페이스에 정적 계약 정의
        Print.Line($"Circle2.TypeName (static abstract): {Circle2.TypeName}");
        Print.Line($"Square.TypeName  (static abstract): {Square.TypeName}");
    }

    // ── 3-2. 연산자 오버로딩 ────────────────────────────────────────────────
    static void ShowOperatorOverloading()
    {
        Print.Section("3-2. 연산자 오버로딩 — Vector2D");

        var v1 = new Vector2D(3, 4);
        var v2 = new Vector2D(1, 2);

        Print.Line($"v1 = {v1}");
        Print.Line($"v2 = {v2}");
        Print.Line($"v1 + v2 = {v1 + v2}");
        Print.Line($"v1 - v2 = {v1 - v2}");
        Print.Line($"v1 * 2  = {v1 * 2}");
        Print.Line($"2  * v1 = {2 * v1}");
        Print.Line($"v1 / 2  = {v1 / 2}");
        Print.Line($"-v1     = {-v1}");
        Print.Line($"|v1|    = {v1.Magnitude:F3}");
        Print.Line($"v1 == v1: {v1 == new Vector2D(3, 4)}");
        Print.Line($"v1 != v2: {v1 != v2}");
        Print.Line($"v1 > v2 (magnitude): {v1 > v2}");
    }

    // ── 3-3. 변환 연산자 ────────────────────────────────────────────────────
    static void ShowConversionOperators()
    {
        Print.Section("3-3. 변환 연산자 — implicit / explicit");

        // implicit: 명시적 캐스팅 없이 자동 변환
        Vector2D v = new(3.0, 4.0);           // double tuple → Vector2D (implicit)
        Print.Line($"implicit (3.0,4.0) → Vector2D: {v}");

        (double, double) tuple = v;            // Vector2D → (double,double) (implicit)
        Print.Line($"implicit Vector2D → tuple: {tuple}");

        // explicit: 명시적 캐스팅 필요
        Celsius c = new(100);
        Fahrenheit f = (Fahrenheit)c;          // explicit 변환
        Print.Line($"explicit 100°C → {f.Value:F1}°F");

        Fahrenheit f2 = new(32);
        Celsius c2 = (Celsius)f2;
        Print.Line($"explicit  32°F → {c2.Value:F1}°C");
    }

    // ── 3-4. 인덱서 ─────────────────────────────────────────────────────────
    static void ShowIndexer()
    {
        Print.Section("3-4. 인덱서 — 단일·다중·문자열 키");

        var mat = new Matrix(3, 3);
        mat[0, 0] = 1; mat[1, 1] = 5; mat[2, 2] = 9;
        mat[0, 2] = 3; mat[2, 0] = 7;
        Print.Line($"Matrix[0,0]={mat[0,0]}  [1,1]={mat[1,1]}  [2,2]={mat[2,2]}");
        Print.Line($"Matrix[0,2]={mat[0,2]}  [2,0]={mat[2,0]}");

        // 문자열 키 인덱서
        var config = new Config();
        config["host"] = "localhost";
        config["port"] = "8080";
        Print.Line($"config[\"host\"]={config["host"]}  [\"port\"]={config["port"]}");
        Print.Line($"config[\"missing\"]={config["missing"]}  (없는 키 기본값)");
    }

    // ── 3-5. 타입 캐스팅 ────────────────────────────────────────────────────
    static void ShowTypeCasting()
    {
        Print.Section("3-5. 타입 캐스팅 — is · as · 패턴");

        object[] items = [new Circle2(3), new Square(4), "문자열", 42, null!];

        foreach (var item in items)
        {
            string desc = item switch
            {
                Circle2 { Radius: > 2 } c => $"큰 Circle r={c.Radius}",
                Circle2 c                 => $"작은 Circle r={c.Radius}",
                Square s                  => $"Square side={s.Side}",
                string str                => $"string \"{str}\"",
                int n                     => $"int {n}",
                null                      => "null",
                _                         => "기타",
            };
            Print.Line(desc);
        }

        // is 표현식
        object o = new Circle2(5);
        if (o is Circle2 circle && circle.Radius > 3)
            Print.Line($"is 패턴 + 조건: 반지름 {circle.Radius} > 3");

        // as 연산자: 실패 시 null (예외 없음)
        var sq = o as Square;
        Print.Line($"as Square (실패): {sq?.ToString() ?? "null"}");
    }
}

// ── 인터페이스 정의 ────────────────────────────────────────────────────────

public interface IDrawable
{
    string Draw();

    // default 구현 (C# 8+): 구현 클래스가 재정의 안 해도 사용 가능
    string Info() => $"[{GetType().Name}] 그리기 가능";
}

public interface IResizable
{
    void Resize(double factor);
}

public interface ILoggable
{
    string Log();
}

// static abstract (C# 11): 인터페이스 차원의 정적 계약
public interface INamedType
{
    static abstract string TypeName { get; }
}

// ── 도형 구현 ──────────────────────────────────────────────────────────────

public class Circle2(double radius) : IDrawable, INamedType
{
    public double Radius { get; private set; } = radius;

    public string Draw() => $"○ 반지름={Radius:F1}";

    // static abstract 구현
    public static string TypeName => "원(Circle)";
}

public class Square(double side) : IDrawable, IResizable, ILoggable, INamedType
{
    public double Side { get; private set; } = side;

    public string Draw()   => $"□ 변={Side:F1}";
    public void Resize(double factor) => Side *= factor;

    // 명시적 구현: ILoggable을 통해서만 접근
    string ILoggable.Log() => $"[{DateTime.Now:HH:mm:ss}] Square(side={Side})";

    public static string TypeName => "정사각형(Square)";
}

// ── Vector2D 연산자 오버로딩 ──────────────────────────────────────────────

public readonly struct Vector2D(double x, double y)
{
    public double X { get; } = x;
    public double Y { get; } = y;
    public double Magnitude => Math.Sqrt(X * X + Y * Y);

    // 산술
    public static Vector2D operator +(Vector2D a, Vector2D b) => new(a.X + b.X, a.Y + b.Y);
    public static Vector2D operator -(Vector2D a, Vector2D b) => new(a.X - b.X, a.Y - b.Y);
    public static Vector2D operator *(Vector2D v, double s)   => new(v.X * s,   v.Y * s);
    public static Vector2D operator *(double s,   Vector2D v) => new(v.X * s,   v.Y * s);
    public static Vector2D operator /(Vector2D v, double s)   => new(v.X / s,   v.Y / s);
    public static Vector2D operator -(Vector2D v)             => new(-v.X, -v.Y);

    // 비교 (== 오버로딩 시 != 도 반드시 함께)
    public static bool operator ==(Vector2D a, Vector2D b) => a.X == b.X && a.Y == b.Y;
    public static bool operator !=(Vector2D a, Vector2D b) => !(a == b);
    public static bool operator > (Vector2D a, Vector2D b) => a.Magnitude > b.Magnitude;
    public static bool operator < (Vector2D a, Vector2D b) => a.Magnitude < b.Magnitude;

    public override bool Equals(object? obj) => obj is Vector2D v && this == v;
    public override int  GetHashCode()        => HashCode.Combine(X, Y);

    // implicit: (double,double) ↔ Vector2D
    public static implicit operator Vector2D((double x, double y) t) => new(t.x, t.y);
    public static implicit operator (double, double)(Vector2D v)     => (v.X, v.Y);

    public override string ToString() => $"({X:F1},{Y:F1})";
}

// ── 변환 연산자 ───────────────────────────────────────────────────────────

public readonly record struct Celsius(double Value)
{
    // explicit 변환: Celsius → Fahrenheit
    public static explicit operator Fahrenheit(Celsius c) => new(c.Value * 9 / 5 + 32);
}

public readonly record struct Fahrenheit(double Value)
{
    // explicit 변환: Fahrenheit → Celsius
    public static explicit operator Celsius(Fahrenheit f) => new((f.Value - 32) * 5 / 9);
}

// ── 인덱서 ────────────────────────────────────────────────────────────────

public class Matrix(int rows, int cols)
{
    private readonly double[,] _data = new double[rows, cols];

    // 다중 인덱서
    public double this[int r, int c]
    {
        get => _data[r, c];
        set => _data[r, c] = value;
    }
}

public class Config
{
    private readonly Dictionary<string, string> _store = new();

    // 문자열 키 인덱서
    public string this[string key]
    {
        get => _store.GetValueOrDefault(key, "(기본값)");
        set => _store[key] = value;
    }
}
