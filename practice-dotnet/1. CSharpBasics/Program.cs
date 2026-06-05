using System.Text;
using Microsoft.Extensions.Configuration;

namespace CSharpBasics
{
    internal static class Program
    {
        static int Main(string[] args)
        {
            Console.InputEncoding  = Encoding.UTF8;
            Console.OutputEncoding = Encoding.UTF8;

            // ── 1. Configuration
            Console.WriteLine("=== 1. Configuration ===");
            string environment = Environment.GetEnvironmentVariable("DOTNET_ENVIRONMENT") ?? "Production";
            Console.WriteLine($"현재 실행 환경: {environment}");

            IConfiguration config = new ConfigurationBuilder()
                .SetBasePath(AppContext.BaseDirectory)
                .AddJsonFile("appsettings.json", optional: false, reloadOnChange: true)
                .AddJsonFile($"appsettings.{environment}.json", optional: true)
                .AddEnvironmentVariables()
                .AddCommandLine(args)
                .Build();
            Console.WriteLine($"메시지: {config["Custom:message"]}");

            // ── 2. Records (C# 9+)
            Console.WriteLine("\n=== 2. Records ===");
            var p1 = new Point(3, 4);
            var p2 = p1 with { Y = 0 };  // non-destructive mutation
            Console.WriteLine($"p1={p1}, p2={p2}, 같음={p1 == p2}");
            Console.WriteLine($"p1 거리: {p1.Distance:F2}");

            // ── 3. Pattern Matching (C# 8-12)
            Console.WriteLine("\n=== 3. Pattern Matching ===");
            object[] items = [42, "hello", 3.14, null!, new Point(1, 2), -5];
            foreach (var item in items)
                Console.WriteLine($"  {item ?? "null",-20} → {Classify(item)}");

            // ── 4. TextUtils
            Console.WriteLine("\n=== 4. TextUtils ===");
            Console.WriteLine(TextUtils.TitleCase("hello dotnet world"));
            Console.WriteLine($"palindrome: {TextUtils.IsPalindrome("A man a plan a canal Panama")}");
            Console.WriteLine(TextUtils.Truncate("This is a very long string", 15));
            Console.WriteLine($"단어 수: {TextUtils.CountWords("hello dotnet world")}");

            // ── 5. NumberUtils
            Console.WriteLine("\n=== 5. NumberUtils ===");
            Console.WriteLine($"IsPrime(17): {NumberUtils.IsPrime(17)}");
            Console.WriteLine($"30 이하 소수: {string.Join(", ", NumberUtils.PrimesUpTo(30))}");
            Console.WriteLine($"Fibonacci(10): {NumberUtils.Fibonacci(10)}");
            Console.WriteLine($"피보나치 수열(8개): [{string.Join(", ", NumberUtils.FibonacciSequence(8))}]");

            // ── 6. LINQ 체이닝
            Console.WriteLine("\n=== 6. LINQ ===");
            var result = NumberUtils.PrimesUpTo(50)
                .Where(p => p > 10)
                .Select(p => p * p)
                .Take(5)
                .ToList();
            Console.WriteLine($"11 초과 소수의 제곱 (첫 5개): [{string.Join(", ", result)}]");

            return 0;
        }

        static string Classify(object? obj) => obj switch
        {
            null                            => "null",
            int n when n < 0                => $"음의 정수({n})",
            int n                           => $"양의 정수({n})",
            string { Length: > 5 } s        => $"긴 문자열(len={s.Length})",
            string s                        => $"짧은 문자열({s})",
            double d                        => $"실수({d:F2})",
            Point(var x, var y)             => $"Point({x}, {y})",
            _                               => $"기타({obj.GetType().Name})"
        };
    }

    // Record: 불변 값 타입. 자동 Equals/GetHashCode/ToString, 구조 분해 지원
    public record Point(double X, double Y)
    {
        public double Distance => Math.Sqrt(X * X + Y * Y);
    }

    public static class Mod
    {
        public static int fn() => 1;
    }
}
