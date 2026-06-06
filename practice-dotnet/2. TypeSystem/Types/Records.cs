namespace TypeSystem;

// ──────────────────────────────────────────────────────────────────────────────
// record (= record class) — C# 9+
// 컴파일러 자동 생성: Equals, GetHashCode, ToString, operator==, !=, Deconstruct
// · 참조 타입 (힙 할당)
// · 기본적으로 불변 (positional 프로퍼티는 init-only)
// · `with` 표현식: non-destructive mutation (새 인스턴스 반환)
// ──────────────────────────────────────────────────────────────────────────────

// 1. Positional record — 간결한 선언, 모든 멤버 자동 생성
public record PersonRecord(string Name, int Age);

// 2. Positional record + 추가 멤버 + 유효성 검사 (Static factory 패턴)
//    record에서 생성 시 유효성 검사가 필요할 때 권장되는 패턴:
//    ① 생성자를 private으로 막고 static factory로만 생성 허용
//    ② 또는 C# 9의 compact constructor `public Product { ... }` 사용
public record Product(string Id, string Name, decimal Price)
{
    // 추가 계산 속성
    public string DisplayName => $"[{Id}] {Name}";

    // Static factory: 유효성 검사 후 인스턴스 생성
    public static Product Create(string id, string name, decimal price)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(id,   nameof(id));
        ArgumentOutOfRangeException.ThrowIfNegative(price, nameof(price));
        return new Product(id, name, price);
    }

    public Product WithDiscount(double pct) => this with { Price = Price * (decimal)(1 - pct) };
}

// 3. Non-positional record — 프로퍼티를 명시적으로 선언
public record Address
{
    public required string Street  { get; init; }
    public required string City    { get; init; }
    public string Country { get; init; } = "Korea";

    public string Full => $"{Street}, {City}, {Country}";
}

// 4. Record 상속 — record는 record class만 상속 가능
public record Animal(string Name, string Species);

public record Dog(string Name, string Breed)
    : Animal(Name, "Canis lupus familiaris")
{
    public string Bark() => $"{Name}: 멍멍!";
}

// 5. sealed record — 상속 차단 + 값 동등성
public sealed record Point2D(double X, double Y)
{
    public static Point2D Origin => new(0, 0);
    public double Distance => Math.Sqrt(X * X + Y * Y);
    public Point2D Translate(double dx, double dy) => this with { X = X + dx, Y = Y + dy };
    public double DistanceTo(Point2D other) =>
        Math.Sqrt(Math.Pow(X - other.X, 2) + Math.Pow(Y - other.Y, 2));
}

// 6. record struct (C# 10+) — 값 타입 레코드, 스택 할당
public record struct Velocity(double Vx, double Vy)
{
    public double Speed => Math.Sqrt(Vx * Vx + Vy * Vy);
    public Velocity Scale(double factor) => this with { Vx = Vx * factor, Vy = Vy * factor };
}

// 7. readonly record struct — 불변 값 타입 레코드
public readonly record struct RGB(byte R, byte G, byte B)
{
    public static readonly RGB Black = new(0,   0,   0);
    public static readonly RGB White = new(255, 255, 255);
    public static readonly RGB Red   = new(255, 0,   0);
    public static readonly RGB Green = new(0,   255, 0);
    public static readonly RGB Blue  = new(0,   0,   255);

    public string Hex => $"#{R:X2}{G:X2}{B:X2}";

    public RGB Blend(RGB other) => new(
        (byte)((R + other.R) / 2),
        (byte)((G + other.G) / 2),
        (byte)((B + other.B) / 2));

    public RGB Invert() => new((byte)(255 - R), (byte)(255 - G), (byte)(255 - B));
}
