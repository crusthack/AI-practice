namespace TypeSystem;

// ──────────────────────────────────────────────────────────────────────────────
// 1. 기본 struct — 가변(mutable), 필드 직접 노출
//    · 스택(로컬 변수) 또는 포함 타입 내부에 inline 저장 → 힙 할당 없음
//    · 기본 Equals는 reflection 기반(느림) → IEquatable<T> 직접 구현 권장
//    · 복사 의미론: 대입/인수 전달 시 전체 필드가 복사됨
// ──────────────────────────────────────────────────────────────────────────────
public struct MutablePoint : IEquatable<MutablePoint>
{
    public double X;
    public double Y;

    public MutablePoint(double x, double y) { X = x; Y = y; }

    public double Distance => Math.Sqrt(X * X + Y * Y);

    public bool Equals(MutablePoint other) => X == other.X && Y == other.Y;
    public override bool Equals(object? obj) => obj is MutablePoint p && Equals(p);
    public override int GetHashCode() => HashCode.Combine(X, Y);
    public static bool operator ==(MutablePoint a, MutablePoint b) => a.Equals(b);
    public static bool operator !=(MutablePoint a, MutablePoint b) => !a.Equals(b);
    public override string ToString() => $"MutablePoint({X}, {Y})";
}

// ──────────────────────────────────────────────────────────────────────────────
// 2. readonly struct — 모든 필드/프로퍼티가 readonly
//    · 컴파일러가 방어적 복사(defensive copy)를 생성하지 않음 → 더 빠름
//    · in 파라미터로 전달 시 실제 복사 없이 참조로 전달
//    · 메서드가 this를 변경할 수 없으므로 스레드 안전성 기본 보장
// ──────────────────────────────────────────────────────────────────────────────
public readonly struct ImmutablePoint : IEquatable<ImmutablePoint>
{
    public double X { get; }
    public double Y { get; }

    public ImmutablePoint(double x, double y) { X = x; Y = y; }

    public double Distance => Math.Sqrt(X * X + Y * Y);

    // 불변이므로 변환은 항상 새 인스턴스를 반환
    public ImmutablePoint Translate(double dx, double dy) => new(X + dx, Y + dy);
    public ImmutablePoint Scale(double factor) => new(X * factor, Y * factor);

    public bool Equals(ImmutablePoint other) => X == other.X && Y == other.Y;
    public override bool Equals(object? obj) => obj is ImmutablePoint p && Equals(p);
    public override int GetHashCode() => HashCode.Combine(X, Y);
    public static bool operator ==(ImmutablePoint a, ImmutablePoint b) => a.Equals(b);
    public static bool operator !=(ImmutablePoint a, ImmutablePoint b) => !a.Equals(b);
    public override string ToString() => $"ImmutablePoint({X}, {Y})";
}

// ──────────────────────────────────────────────────────────────────────────────
// 3. record struct (C# 10+) — 값 의미론 + 컴파일러 자동 생성
//    자동 생성: Equals, GetHashCode, ToString, operator==, !=, Deconstruct
//    · `with` 표현식으로 non-destructive mutation 지원
//    · 기본적으로 가변(mutable) — 필요시 readonly record struct 사용
// ──────────────────────────────────────────────────────────────────────────────
public record struct RecordPoint(double X, double Y)
{
    public double Distance => Math.Sqrt(X * X + Y * Y);
    public RecordPoint Translate(double dx, double dy) => this with { X = X + dx, Y = Y + dy };
}

// ──────────────────────────────────────────────────────────────────────────────
// 4. readonly record struct — 불변 값 타입 레코드
//    · readonly struct의 성능 + record의 편의 문법 결합
//    · `with` 표현식은 여전히 사용 가능 (새 인스턴스 생성)
// ──────────────────────────────────────────────────────────────────────────────
public readonly record struct Temperature(double Celsius)
{
    public double Fahrenheit => Celsius * 9.0 / 5.0 + 32.0;
    public double Kelvin     => Celsius + 273.15;

    public static Temperature FromFahrenheit(double f) => new((f - 32.0) * 5.0 / 9.0);
    public static Temperature FromKelvin(double k)     => new(k - 273.15);

    public Temperature Add(double delta) => this with { Celsius = Celsius + delta };
}

// ──────────────────────────────────────────────────────────────────────────────
// 5. ref struct (C# 7.2+) — 스택 전용 구조체
//    · 힙에 올라갈 수 없음: 박싱 불가, 일반 class/struct 필드로 사용 불가
//    · Span<T>, ReadOnlySpan<T>가 이 방식으로 구현됨
//    · unsafe 없이 unmanaged 포인터/연속 메모리를 안전하게 래핑
// ──────────────────────────────────────────────────────────────────────────────
public ref struct SpanWrapper
{
    private Span<int> _data;

    public SpanWrapper(Span<int> data) { _data = data; }

    public int Length => _data.Length;
    public ref int this[int i] => ref _data[i];

    public int Sum()
    {
        int s = 0;
        foreach (var v in _data) s += v;
        return s;
    }

    public void Fill(int value) => _data.Fill(value);
}

// ──────────────────────────────────────────────────────────────────────────────
// 6. enum — 정수 기반 명명된 상수
//    · 기본 underlying type: int. byte/short/long 지정 가능
//    · [Flags]: 비트 플래그 조합을 위한 패턴
// ──────────────────────────────────────────────────────────────────────────────
public enum Direction : byte { North, East, South, West }

[Flags]
public enum FilePermission
{
    None      = 0,
    Read      = 1 << 0,  // 1
    Write     = 1 << 1,  // 2
    Execute   = 1 << 2,  // 4
    ReadWrite = Read | Write,
    All       = Read | Write | Execute,
}
