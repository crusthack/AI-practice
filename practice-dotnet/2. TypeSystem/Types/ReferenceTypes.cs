namespace TypeSystem;

// ──────────────────────────────────────────────────────────────────────────────
// 1. 기본 class — 가변, 참조 의미론
//    · 항상 힙 할당, GC 관리
//    · 대입 시 참조(주소)만 복사 → 같은 객체를 가리킴
//    · 기본 Equals = ReferenceEquals (주소 비교)
// ──────────────────────────────────────────────────────────────────────────────
public class PersonClass
{
    public string Name { get; set; }
    public int Age { get; set; }

    public PersonClass(string name, int age)
    {
        Name = name;
        Age  = age;
    }

    public override string ToString() => $"Person({Name}, {Age})";
}

// ──────────────────────────────────────────────────────────────────────────────
// 2. Primary constructor (C# 12) — 타입 선언과 생성자를 한 줄에
//    · 매개변수가 클래스 전체 스코프에서 캡처됨
//    · struct / record / interface 에도 적용 가능
// ──────────────────────────────────────────────────────────────────────────────
public class PersonV2(string name, int age)
{
    public string Name { get; } = name;
    public int Age    { get; } = age;

    // primary ctor 매개변수를 메서드 내에서 직접 참조
    public string Greet() => $"안녕하세요, {name}입니다. ({age}세)";
    public override string ToString() => $"PersonV2({Name}, {Age})";
}

// ──────────────────────────────────────────────────────────────────────────────
// 3. init-only + required (C# 9 / C# 11)
//    · init: object initializer 또는 생성자에서만 설정 가능 → 이후 불변
//    · required: 객체 생성 시 반드시 설정해야 함 (컴파일 타임 강제)
// ──────────────────────────────────────────────────────────────────────────────
public class ServerConfig
{
    public required string Host    { get; init; }
    public required int    Port    { get; init; }
    public bool   UseTls    { get; init; } = true;
    public int    TimeoutMs { get; init; } = 30_000;

    public override string ToString() =>
        $"ServerConfig({Host}:{Port}, tls={UseTls}, timeout={TimeoutMs}ms)";
}

// ──────────────────────────────────────────────────────────────────────────────
// 4. sealed class — 상속 불가
//    · JIT 컴파일러가 가상 디스패치를 devirtualize 최적화할 수 있음
//    · 의도적 "확장 금지"를 명시적으로 표현
// ──────────────────────────────────────────────────────────────────────────────
public sealed class ApiToken
{
    private readonly string _value;

    public ApiToken(string value) => _value = value;

    public bool IsValid => !string.IsNullOrWhiteSpace(_value) && _value.Length >= 16;
    public override string ToString() =>
        $"Token({_value[..Math.Min(4, _value.Length)]}***)";
}

// ──────────────────────────────────────────────────────────────────────────────
// 5. abstract class + 상속 계층
//    · abstract: 인스턴스화 불가, 파생 클래스가 구현 강제
//    · virtual: 파생 클래스가 선택적으로 재정의
//    · override: 부모의 virtual/abstract 멤버를 재정의
// ──────────────────────────────────────────────────────────────────────────────
public abstract class Shape
{
    public abstract double Area      { get; }
    public abstract double Perimeter { get; }

    public virtual string Describe() =>
        $"{GetType().Name}: 넓이={Area:F2}, 둘레={Perimeter:F2}";
}

public sealed class Circle(double radius) : Shape
{
    public double Radius { get; } = radius;
    public override double Area      => Math.PI * Radius * Radius;
    public override double Perimeter => 2 * Math.PI * Radius;
}

public sealed class Rectangle(double width, double height) : Shape
{
    public double Width  { get; } = width;
    public double Height { get; } = height;
    public override double Area      => Width * Height;
    public override double Perimeter => 2 * (Width + Height);
}

public sealed class Triangle(double a, double b, double c) : Shape
{
    // 헤론의 공식
    public override double Area
    {
        get
        {
            double s = (a + b + c) / 2;
            return Math.Sqrt(s * (s - a) * (s - b) * (s - c));
        }
    }
    public override double Perimeter => a + b + c;
}

// ──────────────────────────────────────────────────────────────────────────────
// 6. static class — 인스턴스화 불가, 확장 메서드·유틸리티 전용
//    · 모든 멤버가 static이어야 함
//    · 확장 메서드(extension method)의 컨테이너로 주로 사용
// ──────────────────────────────────────────────────────────────────────────────
public static class ShapeExtensions
{
    public static double ScaledArea(this Shape s, double factor) =>
        s.Area * factor * factor;

    public static bool IsLargerThan(this Shape a, Shape b) =>
        a.Area > b.Area;

    public static Shape Largest(this IEnumerable<Shape> shapes) =>
        shapes.MaxBy(s => s.Area)!;
}

// ──────────────────────────────────────────────────────────────────────────────
// 7. Generic class — 타입 파라미터로 재사용성 극대화
//    · where T : ... 제약으로 사용 가능한 타입 범위 한정
// ──────────────────────────────────────────────────────────────────────────────
public class Result<T>
{
    public T?     Value    { get; }
    public string? Error   { get; }
    public bool   IsSuccess => Error is null;

    private Result(T? value, string? error) { Value = value; Error = error; }

    public static Result<T> Ok(T value)       => new(value, null);
    public static Result<T> Fail(string error) => new(default, error);

    public Result<TNext> Map<TNext>(Func<T, TNext> transform) =>
        IsSuccess
            ? Result<TNext>.Ok(transform(Value!))
            : Result<TNext>.Fail(Error!);

    public T GetOrThrow() =>
        IsSuccess ? Value! : throw new InvalidOperationException(Error);

    public override string ToString() =>
        IsSuccess ? $"Ok({Value})" : $"Fail({Error})";
}

// ──────────────────────────────────────────────────────────────────────────────
// 8. Nested class — 외부 타입과 논리적으로 강하게 결합된 타입
//    · 외부 클래스의 private 멤버에 접근 가능
//    · 네임스페이스 오염 방지
// ──────────────────────────────────────────────────────────────────────────────
public class Order
{
    public int          Id    { get; init; }
    public List<LineItem> Items { get; } = new();
    public decimal      Total => Items.Sum(i => i.Subtotal);

    public sealed class LineItem(string name, decimal price, int qty)
    {
        public string  Name     { get; } = name;
        public decimal Price    { get; } = price;
        public int     Quantity { get; } = qty;
        public decimal Subtotal => Price * Quantity;
        public override string ToString() => $"{Name} × {Quantity} = {Subtotal:C}";
    }
}
