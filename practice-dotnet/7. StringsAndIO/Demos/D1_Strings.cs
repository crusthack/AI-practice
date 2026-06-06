using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;

namespace StringsAndIO.Demos;

static class D1_Strings
{
    public static void Run()
    {
        Print.Header("1. String · StringBuilder · Span<char>");
        ShowStringInternals();
        ShowStringBuilder();
        ShowStringFormatting();
        ShowStringSearch();
        ShowRegex();
        ShowSpanChar();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowStringInternals()
    {
        Print.Section("1-1. string 불변성 · 인터닝");

        string a = "hello";
        string b = "hello";
        string c = new string("hello".ToCharArray()); // 인터닝 없는 새 인스턴스

        Print.Line($"a == b (값):          {a == b}");
        Print.Line($"ReferenceEquals(a,b): {ReferenceEquals(a, b)}  ← 인터닝으로 같은 참조");
        Print.Line($"ReferenceEquals(a,c): {ReferenceEquals(a, c)}  ← 다른 인스턴스");

        // string.Intern 수동 인터닝
        string d = string.Intern(c);
        Print.Line($"ReferenceEquals(a, Intern(c)): {ReferenceEquals(a, d)}");

        // 불변성 — 모든 연산은 새 문자열 반환
        string s = "Hello";
        string upper = s.ToUpper(); // 새 문자열
        Print.Line($"원본: '{s}'  ToUpper: '{upper}'  원본 변경 없음: {s == "Hello"}");

        // 문자열 비교 옵션
        Print.Line($"OrdinalIgnoreCase: {"ABC".Equals("abc", StringComparison.OrdinalIgnoreCase)}");
        Print.Line($"CurrentCulture:    {"ä".CompareTo("b")} (독일어 문화권 영향)");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowStringBuilder()
    {
        Print.Section("1-2. StringBuilder — 가변 문자열 구성");

        var sb = new StringBuilder(capacity: 64);
        sb.Append("Hello");
        sb.Append(", ");
        sb.Append("World");
        sb.AppendLine("!");
        sb.Insert(5, " Beautiful");
        sb.Replace("Beautiful ", "");
        Print.Line($"결과: '{sb}'");
        Print.Line($"Length: {sb.Length}  Capacity: {sb.Capacity}");

        // 대량 연결 — StringBuilder vs string 연결 비교
        var sbLoop = new StringBuilder();
        for (int i = 0; i < 1000; i++)
            sbLoop.Append(i).Append(',');
        sbLoop.Length--; // 마지막 쉼표 제거
        Print.Line($"루프 1000개 → 길이: {sbLoop.Length}");

        // AppendFormat, AppendJoin
        var sb2 = new StringBuilder();
        sb2.AppendFormat("합계: {0:N0}원", 1_500_000);
        sb2.AppendLine();
        sb2.AppendJoin(" | ", new[] { "A", "B", "C" });
        Print.Line($"Format+Join: {sb2}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowStringFormatting()
    {
        Print.Section("1-3. 문자열 포매팅 — 보간 · 형식 지정자 · 다국어");

        double price = 1_234_567.89;
        DateTime now = new(2024, 6, 15, 14, 30, 0);

        // 표준 형식 지정자
        Print.Line($"  C (통화):   {price:C}");
        Print.Line($"  N2 (숫자):  {price:N2}");
        Print.Line($"  E (지수):   {price:E3}");
        Print.Line($"  날짜 d:     {now:d}");
        Print.Line($"  날짜 yyyy-MM-dd HH:mm: {now:yyyy-MM-dd HH:mm}");
        Print.Line($"  상대 경과:  {(DateTime.UtcNow - now.ToUniversalTime()).Days}일 전");

        // CultureInfo
        var ci = CultureInfo.GetCultureInfo("ko-KR");
        Print.Line($"  ko-KR C:  {price.ToString("C", ci)}");

        // 정렬 보간 — 패딩
        string[] headers = ["이름", "점수", "등급"];
        string[] data    = ["홍길동", "98.5", "A+"];
        foreach (var (h, v) in headers.Zip(data))
            Console.Write($"  {h,6}: {v,-8}");
        Console.WriteLine();

        // FormattableString / Invariant culture
        FormattableString fs = $"가격: {price:F2}";
        Print.Line($"  Invariant: {FormattableString.Invariant(fs)}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowStringSearch()
    {
        Print.Section("1-4. 검색 · 분리 · 변환");

        const string text = "Hello, World! Hello, .NET!";

        Print.Line($"Contains:        {text.Contains("World")}");
        Print.Line($"StartsWith:      {text.StartsWith("Hello")}");
        Print.Line($"IndexOf 두 번째: {text.IndexOf("Hello", 1)}");
        Print.Line($"LastIndexOf:     {text.LastIndexOf("Hello")}");

        // Split
        var words = text.Split([',', '!', ' '], StringSplitOptions.RemoveEmptyEntries);
        Print.Items(words, "  단어: ");

        // 문자열 처리
        Print.Line($"Trim:     '  hello  '.Trim() = '{"  hello  ".Trim()}'");
        Print.Line($"PadLeft:  {"42".PadLeft(6, '0')}");
        Print.Line($"Substring:{text[7..12]}  (range 연산자)");

        // string.Join / Concat / Concat
        var parts = new[] { "A", "B", "C" };
        Print.Line($"Join:   {string.Join("-", parts)}");
        Print.Line($"Concat: {string.Concat(parts)}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowRegex()
    {
        Print.Section("1-5. 정규식 — Regex");

        // 컴파일된 소스 제너레이터 패턴 (GeneratedRegex는 partial 클래스 필요 → 런타임 버전)
        var emailRx = new Regex(@"^[\w.]+@[\w.]+\.[a-z]{2,}$",
            RegexOptions.IgnoreCase | RegexOptions.Compiled);

        string[] candidates = ["user@example.com", "invalid-email", "test.123@sub.domain.org"];
        foreach (var c in candidates)
            Print.Line($"  '{c}' → {(emailRx.IsMatch(c) ? "유효" : "무효")}");

        // 그룹 캡처
        var dateRx = new Regex(@"(?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2})");
        var m = dateRx.Match("오늘은 2024-06-15 입니다.");
        if (m.Success)
        {
            Print.Line($"  year={m.Groups["year"].Value}  month={m.Groups["month"].Value}  day={m.Groups["day"].Value}");
        }

        // Replace with MatchEvaluator
        string masked = Regex.Replace("전화: 010-1234-5678, 010-9876-5432",
            @"\d{3}-\d{4}-\d{4}",
            m => m.Value[..3] + "-****-****");
        Print.Line($"  마스킹: {masked}");

        // Span 기반 빠른 검사 (Regex.IsMatch)
        ReadOnlySpan<char> span = "hello@world.com".AsSpan();
        Print.Line($"  Span IsMatch: {emailRx.IsMatch(span)}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowSpanChar()
    {
        Print.Section("1-6. Span<char> · MemoryExtensions — 무할당 문자열 처리");

        string csv = "홍길동,25,서울,개발자";
        ReadOnlySpan<char> span = csv.AsSpan();

        // 할당 없이 슬라이싱
        int idx = 0;
        while (true)
        {
            int comma = span.IndexOf(',');
            ReadOnlySpan<char> token = comma >= 0 ? span[..comma] : span;
            Console.Write($"  [{token}]");
            if (comma < 0) break;
            span = span[(comma + 1)..];
            idx++;
        }
        Console.WriteLine();

        // MemoryExtensions: Trim, Contains, StartsWith on Span
        ReadOnlySpan<char> padded = "  hello  ".AsSpan();
        Print.Line($"  Trim: '{padded.Trim()}'");
        Print.Line($"  Contains 'ell': {padded.Contains("ell", StringComparison.Ordinal)}");

        // stackalloc 과 Span — 소문자 변환 무할당
        Span<char> buf = stackalloc char[csv.Length];
        csv.AsSpan().ToLowerInvariant(buf);
        Print.Line($"  ToLower(stackalloc): {new string(buf)}");
    }
}
