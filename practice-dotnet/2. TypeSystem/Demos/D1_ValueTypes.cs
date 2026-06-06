namespace TypeSystem.Demos;

static class D1_ValueTypes
{
    public static void Run()
    {
        Print.Header("1. 값 타입 (Value Types)");

        // ── 1-1. struct 복사 의미론
        Print.Section("1-1. struct — 복사 의미론 (copy semantics)");
        {
            var a = new MutablePoint(1, 2);
            var b = a;       // 전체 필드 복사 — a와 b는 독립적인 복사본
            b.X = 99;
            Console.WriteLine($"    a={a}");
            Console.WriteLine($"    b={b}  (b.X=99로 변경해도 a는 영향 없음)");
            Console.WriteLine($"    a == b: {a == b}");

            // 메서드에 전달할 때도 복사
            Mutate(a);
            Console.WriteLine($"    Mutate(a) 후 a={a}  (변경 없음)");
        }

        // ── 1-2. readonly struct — 방어적 복사 없음
        Print.Section("1-2. readonly struct — 방어적 복사(defensive copy) 제거");
        {
            var p = new ImmutablePoint(3, 4);
            Console.WriteLine($"    p={p}  거리={p.Distance:F2}");

            var moved = p.Translate(1, 1);
            Console.WriteLine($"    Translate(+1,+1): {moved}");
            Console.WriteLine($"    원본 p={p}  (불변)");

            // in 파라미터: readonly struct를 참조로 전달 (복사 없음)
            // mutable struct를 in으로 전달하면 방어적 복사가 발생한다
            Console.WriteLine($"    in 전달 후 거리: {GetDistance(in p):F2}  (복사 없음)");

            // readonly struct는 `in` 없이 전달해도 컴파일러가 최적화할 수 있음
            Console.WriteLine($"    p1 == p2: {p == new ImmutablePoint(3, 4)}");
        }

        // ── 1-3. record struct
        Print.Section("1-3. record struct — 컴파일러 자동 생성 (C# 10+)");
        {
            var r1 = new RecordPoint(1, 2);
            var r2 = new RecordPoint(1, 2);
            var r3 = r1 with { Y = 99 };   // non-destructive mutation

            Console.WriteLine($"    r1={r1}  r2={r2}");
            Console.WriteLine($"    r1 == r2: {r1 == r2}   (값 동등성, 자동 생성)");
            Console.WriteLine($"    with Y=99: r3={r3}");
            Console.WriteLine($"    r1={r1}  (원본 유지)");

            // Deconstruct 자동 생성
            var (x, y) = r1;
            Console.WriteLine($"    구조 분해: x={x}, y={y}");

            // record struct는 기본적으로 가변
            r3.X = -1;
            Console.WriteLine($"    r3.X=-1 후: {r3}  (가변)");
        }

        // ── 1-4. readonly record struct
        Print.Section("1-4. readonly record struct — 불변 값 타입 레코드 (C# 10+)");
        {
            var t1 = new Temperature(100);
            Console.WriteLine($"    {t1.Celsius}°C = {t1.Fahrenheit:F1}°F = {t1.Kelvin:F2}K");

            var t2 = Temperature.FromFahrenheit(212);
            Console.WriteLine($"    212°F → {t2.Celsius:F1}°C");

            var t3 = t1.Add(-10);
            Console.WriteLine($"    with -10°C: {t3}  원본: {t1}  (불변)");

            // readonly → 필드 변경 불가
            // t1.Celsius = 50; // 컴파일 에러
        }

        // ── 1-5. ref struct — 스택 전용
        Print.Section("1-5. ref struct — 스택 전용 (Span<T> 기반 구조)");
        {
            // stackalloc: 스택에 연속 메모리 할당 (GC 부담 없음)
            Span<int> raw = stackalloc int[] { 10, 20, 30, 40, 50 };
            var wrapper = new SpanWrapper(raw);

            Console.WriteLine($"    길이={wrapper.Length}  합계={wrapper.Sum()}");

            wrapper[0] = 99;   // ref 반환 → 직접 수정
            Console.WriteLine($"    [0]=99 후 합계={wrapper.Sum()}");

            // ref struct는 박싱 불가 (object에 대입 불가)
            // object o = wrapper; // 컴파일 에러

            // Span<T> 직접 사용 예시
            Span<char> chars = stackalloc char[] { 'H', 'e', 'l', 'l', 'o' };
            Console.WriteLine($"    stackalloc char: {new string(chars)}");
        }

        // ── 1-6. enum & [Flags]
        Print.Section("1-6. enum & [Flags] enum");
        {
            Direction d = Direction.North;
            Console.WriteLine($"    Direction: {d} = {(byte)d}");

            // Flags: 비트 OR로 여러 값 조합
            var perm = FilePermission.Read | FilePermission.Write;
            Console.WriteLine($"    권한: {perm}  (값={perm:D})");
            Console.WriteLine($"    읽기: {perm.HasFlag(FilePermission.Read)}  " +
                              $"실행: {perm.HasFlag(FilePermission.Execute)}");

            // Enum.Parse / Enum.TryParse
            if (Enum.TryParse<Direction>("South", out var parsed))
                Console.WriteLine($"    파싱: {parsed}");

            // 모든 값 열거
            Console.WriteLine($"    모든 Direction: {string.Join(", ", Enum.GetNames<Direction>())}");
        }
    }

    static void Mutate(MutablePoint p)
    {
        p.X = 999;  // 복사본을 변경 — 호출자에게 영향 없음
    }

    static double GetDistance(in ImmutablePoint p) => p.Distance;
}
