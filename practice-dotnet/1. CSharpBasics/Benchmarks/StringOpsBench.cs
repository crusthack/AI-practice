using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using CSharpBasics;

// ─────────────────────────────────────────────────────
// 문자열 연결 방식별 성능 비교
//
// += 연결     : 매 반복마다 새 문자열 할당 → O(n²) GC 압력.
// StringBuilder: 내부 버퍼 재사용 → O(n) 할당.
// string.Join : BCL 최적화된 단일 호출.
// ─────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class StringOpsBench
{
    [Params(100, 10_000)]
    public int Iterations;

    private static readonly string TestWord =
        "hello dotnet world example string for benchmarking";

    [Benchmark(Baseline = true)]
    public string StringConcat()
    {
        string result = "";
        for (int i = 0; i < Iterations; i++)
            result += i + ",";
        return result;
    }

    [Benchmark]
    public string StringBuilder()
    {
        var sb = new System.Text.StringBuilder();
        for (int i = 0; i < Iterations; i++)
            sb.Append(i).Append(',');
        return sb.ToString();
    }

    [Benchmark]
    public string JoinInts() =>
        string.Join(',', Enumerable.Range(0, Iterations));

    [Benchmark]
    public string TitleCase() => TextUtils.TitleCase(TestWord);

    [Benchmark]
    public bool IsPalindrome() =>
        TextUtils.IsPalindrome("A man a plan a canal Panama");

    [Benchmark]
    public string Truncate() => TextUtils.Truncate(TestWord, 20);
}
