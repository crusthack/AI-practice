namespace TypeSystem.Demos;

static class D2_ReferenceTypes
{
    public static void Run()
    {
        Print.Header("2. 참조 타입 (Reference Types)");

        // ── 2-1. class — 참조 의미론
        Print.Section("2-1. class — 참조 의미론 (reference semantics)");
        {
            var a = new PersonClass("Alice", 30);
            var b = a;      // 참조 복사 — a와 b가 같은 객체를 가리킴
            b.Age = 99;

            Console.WriteLine($"    a={a}");
            Console.WriteLine($"    b={b}  (b.Age=99 변경 시 a도 바뀜)");
            Console.WriteLine($"    ReferenceEquals(a, b): {ReferenceEquals(a, b)}  (같은 인스턴스)");

            // 기본 Equals = 참조 비교
            var c = new PersonClass("Alice", 99);
            Console.WriteLine($"    a.Equals(c): {a.Equals(c)}  (내용 같아도 다른 인스턴스 → false)");
        }

        // ── 2-2. Primary constructor (C# 12)
        Print.Section("2-2. Primary constructor (C# 12)");
        {
            var p = new PersonV2("Bob", 25);
            Console.WriteLine($"    {p}");
            Console.WriteLine($"    {p.Greet()}");
        }

        // ── 2-3. init-only + required
        Print.Section("2-3. init-only + required (C# 9 / 11)");
        {
            var cfg = new ServerConfig
            {
                Host    = "api.example.com",
                Port    = 443,
                UseTls  = true,
                TimeoutMs = 10_000,
            };
            Console.WriteLine($"    {cfg}");

            // 생성 후 변경 시도 → 컴파일 에러
            // cfg.Host = "other";  // CS8852: init-only property

            // required 누락 시 → 컴파일 에러
            // var bad = new ServerConfig { Port = 80 };  // CS9035: required member 'Host' not set
        }

        // ── 2-4. sealed class
        Print.Section("2-4. sealed class — 상속 차단 + JIT devirtualize 최적화");
        {
            var token = new ApiToken("abcdef1234567890");
            Console.WriteLine($"    {token}  유효: {token.IsValid}");

            var short_ = new ApiToken("abc");
            Console.WriteLine($"    짧은 토큰: {short_}  유효: {short_.IsValid}");

            // sealed class는 상속 불가
            // class MyToken : ApiToken { }  // CS0509: cannot derive from sealed type
        }

        // ── 2-5. abstract class + 다형성
        Print.Section("2-5. abstract class + 다형성 (polymorphism)");
        {
            Shape[] shapes =
            [
                new Circle(5),
                new Rectangle(4, 6),
                new Triangle(3, 4, 5),
            ];

            foreach (var s in shapes)
                Console.WriteLine($"    {s.Describe()}");

            // 확장 메서드
            Console.WriteLine($"\n    가장 큰 도형: {shapes.Largest().GetType().Name}");
            Console.WriteLine($"    Circle 2배 크기 넓이: {shapes[0].ScaledArea(2):F2}");
            Console.WriteLine($"    Circle > Rectangle: {shapes[0].IsLargerThan(shapes[1])}");

            // 패턴 매칭으로 타입 분기
            foreach (var s in shapes)
            {
                string extra = s switch
                {
                    Circle   c => $"반지름={c.Radius}",
                    Rectangle r => $"가로={r.Width}, 세로={r.Height}",
                    _           => "기타",
                };
                Console.WriteLine($"    {s.GetType().Name}: {extra}");
            }
        }

        // ── 2-6. Generic class
        Print.Section("2-6. Generic class — Result<T> 모나드 패턴");
        {
            var ok   = Result<int>.Ok(42);
            var fail = Result<int>.Fail("값을 찾을 수 없음");

            Console.WriteLine($"    성공: {ok}   실패: {fail}");

            // Map: 성공 시 변환, 실패 시 에러 전파
            var doubled = ok.Map(v => v * 2);
            var nothing = fail.Map(v => v * 2);
            Console.WriteLine($"    Map ×2: {doubled}   실패 전파: {nothing}");

            // 체이닝
            var result = Result<string>.Ok("42")
                .Map(int.Parse)
                .Map(n => n * n);
            Console.WriteLine($"    체인: \"42\" → Parse → ×자신 = {result}");
        }

        // ── 2-7. Nested class
        Print.Section("2-7. Nested class — Order.LineItem");
        {
            var order = new Order { Id = 1001 };
            order.Items.Add(new Order.LineItem("사과", 1_500m, 3));
            order.Items.Add(new Order.LineItem("배",   2_000m, 2));
            order.Items.Add(new Order.LineItem("포도", 5_000m, 1));

            foreach (var item in order.Items)
                Console.WriteLine($"    {item}");
            Console.WriteLine($"    주문#{order.Id} 합계: {order.Total:C}");
        }
    }
}
