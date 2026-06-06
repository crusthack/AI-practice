namespace Iteration.Demos;

static class D2_Conditionals
{
    public static void Run()
    {
        Print.Header("2. 조건문 & 패턴 매칭");

        // ── 2-1. if / else if / else ──────────────────────────────────────────
        Print.Section("2-1. if / else if / else");
        {
            static string Classify(int score)
            {
                if (score >= 90)      return "A";
                else if (score >= 80) return "B";
                else if (score >= 70) return "C";
                else if (score >= 60) return "D";
                else                  return "F";
            }
            Console.WriteLine($"    95→{Classify(95)}, 82→{Classify(82)}, 55→{Classify(55)}");
        }

        // ── 2-2. 삼항 / null 합병 / null 조건 연산자 ─────────────────────────
        Print.Section("2-2. 삼항(?:) / null합병(??) / null조건(?.) 연산자");
        {
            int x = 7;
            string parity = x % 2 == 0 ? "짝수" : "홀수";
            Console.WriteLine($"    삼항: {x} → {parity}");

            // null 합병
            string? nullable = null;
            string result = nullable ?? "기본값";
            Console.WriteLine($"    ?? : {nullable ?? "기본값"}");

            // null 합병 대입
            nullable ??= "초기화됨";
            Console.WriteLine($"    ??=: {nullable}");

            // null 조건 접근 체인
            string? s = null;
            int? len = s?.Length;
            Console.WriteLine($"    ?.Length on null: {len?.ToString() ?? "null"}");
            Console.WriteLine($"    \"hello\"?.ToUpper(): {"hello"?.ToUpper()}");

            // ?[] — null 조건 인덱서 (값 타입은 int?로 받은 뒤 ?.ToString())
            int[]? arr = null;
            int? arrVal = arr?[0];
            Console.WriteLine($"    ?[0] on null array: {arrVal?.ToString() ?? "null"}");
        }

        // ── 2-3. switch 문 (classic) ─────────────────────────────────────────
        Print.Section("2-3. switch 문 (고전 형태)");
        {
            static string DayType(DayOfWeek d)
            {
                switch (d)
                {
                    case DayOfWeek.Saturday:
                    case DayOfWeek.Sunday:
                        return "주말";
                    case DayOfWeek.Monday:
                        return "월요일";
                    default:
                        return "평일";
                }
            }
            Console.WriteLine($"    Saturday→{DayType(DayOfWeek.Saturday)}");
            Console.WriteLine($"    Monday →{DayType(DayOfWeek.Monday)}");
            Console.WriteLine($"    Wednesday→{DayType(DayOfWeek.Wednesday)}");
        }

        // ── 2-4. switch 식 (C# 8+) ───────────────────────────────────────────
        Print.Section("2-4. switch 식 (expression, C# 8+)");
        {
            static string TrafficLight(string color) => color switch
            {
                "red"    => "정지",
                "yellow" => "주의",
                "green"  => "진행",
                _        => throw new ArgumentException($"알 수 없는 색: {color}"),
            };
            Console.WriteLine($"    red→{TrafficLight("red")}, green→{TrafficLight("green")}");

            // 튜플 패턴
            static string RockPaperScissors(string a, string b) => (a, b) switch
            {
                ("rock",     "scissors") => "A 승",
                ("scissors", "paper")    => "A 승",
                ("paper",    "rock")     => "A 승",
                (var x, var y) when x == y => "무승부",
                _                          => "B 승",
            };
            Console.WriteLine($"    rock vs scissors: {RockPaperScissors("rock", "scissors")}");
            Console.WriteLine($"    paper vs paper:   {RockPaperScissors("paper", "paper")}");
        }

        // ── 2-5. 타입 패턴 / when 절 ─────────────────────────────────────────
        Print.Section("2-5. 타입 패턴 + when 절");
        {
            static string Describe(object? obj) => obj switch
            {
                null                        => "null",
                int n when n < 0            => $"음의 정수({n})",
                int n when n == 0           => "영",
                int n                       => $"양의 정수({n})",
                double d                    => $"실수({d:F2})",
                string { Length: 0 }        => "빈 문자열",
                string s when s.Length > 10 => $"긴 문자열({s.Length}자)",
                string s                    => $"문자열(\"{s}\")",
                _                           => obj.GetType().Name,
            };
            foreach (var v in new object?[] { -3, 0, 42, 3.14, "", "hi", "very long string here", null })
                Console.WriteLine($"    {v ?? "null",-20} → {Describe(v)}");
        }

        // ── 2-6. 속성 패턴 / 위치 패턴 ───────────────────────────────────────
        Print.Section("2-6. 속성 패턴(property) & 위치 패턴(positional)");
        {
            var students = SampleData.Students();

            foreach (var s in students.Take(3))
            {
                string tier = s switch
                {
                    { Gpa: >= 3.7 }              => "우등생",
                    { Gpa: >= 3.0, Grade: >= 3 } => "고학년 우수",
                    { Department: "CS" }         => "CS 학생",
                    _                            => "일반",
                };
                Console.WriteLine($"    {s.Name,-8} → {tier}");
            }

            // 위치 패턴: Deconstruct가 있는 타입
            var point = (3, -2);
            string quadrant = point switch
            {
                ( > 0,  > 0) => "1사분면",
                ( < 0,  > 0) => "2사분면",
                ( < 0,  < 0) => "3사분면",
                ( > 0,  < 0) => "4사분면",
                _            => "축 위",
            };
            Console.WriteLine($"    ({point.Item1},{point.Item2}) → {quadrant}");
        }

        // ── 2-7. 관계·논리 패턴 (C# 9+) ─────────────────────────────────────
        Print.Section("2-7. 관계(relational) / 논리(logical) 패턴");
        {
            static string Category(double temp) => temp switch
            {
                < 0             => "빙점 이하",
                >= 0 and < 10   => "매우 추움",
                >= 10 and < 20  => "서늘함",
                >= 20 and < 30  => "적당함",
                >= 30 and < 40  => "더움",
                _               => "폭염",
            };
            foreach (var t in new[] { -5.0, 5, 15, 25, 35, 42 })
                Console.Write($"    {t}°C→{Category(t)}  ");
            Console.WriteLine();

            // not 패턴
            int[] nums = [1, 2, 3, 4, 5, 6];
            var notEven = nums.Where(n => n is not (2 or 4 or 6)).ToArray();
            Console.Write("    not (2 or 4 or 6): "); Print.Items(notEven, "");
        }

        // ── 2-8. 리스트 패턴 (C# 11+) ────────────────────────────────────────
        Print.Section("2-8. 리스트 패턴 (List patterns, C# 11+)");
        {
            static string Classify(int[] arr) => arr switch
            {
                []              => "빈 배열",
                [var single]    => $"단일 요소: {single}",
                [1, 2, ..]      => "1,2로 시작",
                [.., 9, 10]     => "9,10으로 끝",
                [var first, .., var last] => $"첫={first}, 끝={last}",
            };

            Console.WriteLine($"    []           → {Classify([])}");
            Console.WriteLine($"    [42]         → {Classify([42])}");
            Console.WriteLine($"    [1,2,3,4]    → {Classify([1, 2, 3, 4])}");
            Console.WriteLine($"    [5,6,9,10]   → {Classify([5, 6, 9, 10])}");
            Console.WriteLine($"    [3,4,5,6]    → {Classify([3, 4, 5, 6])}");
        }

        // ── 2-9. is 패턴 식 ──────────────────────────────────────────────────
        Print.Section("2-9. is 패턴 식");
        {
            object? val = "hello";

            if (val is string str && str.Length > 3)
                Console.WriteLine($"    is string: \"{str}\" (길이>{3})");

            if (val is not null)
                Console.WriteLine($"    is not null: true");

            // 중첩 is
            object[] items = [42, "text", 3.14, null!];
            foreach (var item in items)
            {
                if (item is int i)        Console.Write($"    int:{i}  ");
                else if (item is string s) Console.Write($"    str:\"{s}\"  ");
                else if (item is null)    Console.Write($"    null  ");
                else                      Console.Write($"    other  ");
            }
            Console.WriteLine();
        }
    }
}

// 샘플 데이터 팩토리
public static class SampleData
{
    public static IReadOnlyList<Student> Students() =>
    [
        new("Alice",  4, 3.9, "CS"),
        new("Bob",    2, 2.8, "Math"),
        new("Carol",  3, 3.5, "CS"),
        new("Dave",   1, 3.1, "Physics"),
        new("Eve",    4, 3.8, "CS"),
        new("Frank",  2, 2.5, "Math"),
        new("Grace",  3, 3.4, "Biology"),
        new("Henry",  1, 2.9, "CS"),
    ];

    public static IReadOnlyList<Product> Products() =>
    [
        new("노트북",    "전자",   1_200_000m, 15),
        new("마우스",    "전자",      35_000m, 80),
        new("키보드",    "전자",     120_000m, 30),
        new("모니터",    "전자",     450_000m, 0),
        new("책상",      "가구",     350_000m, 10),
        new("의자",      "가구",     250_000m, 5),
        new("스탠드",    "가구",      45_000m, 20),
        new("노트",      "문구",       3_000m, 200),
        new("볼펜",      "문구",       1_500m, 500),
        new("형광펜",    "문구",       2_000m, 0),
    ];
}
