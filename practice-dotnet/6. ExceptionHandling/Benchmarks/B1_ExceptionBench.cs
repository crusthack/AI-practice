using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Running;
using ExceptionHandling.Demos;

BenchmarkRunner.Run<ExceptionBench>();

[MemoryDiagnoser, ShortRunJob]
public class ExceptionBench
{
    [Benchmark(Baseline = true)]
    public string ResultPattern()
    {
        var r = ParseSafe("42");
        return r.Match(ok: v => $"{v}", fail: e => e);
    }

    [Benchmark]
    public string ExceptionPattern()
    {
        try   { return ParseException("42").ToString(); }
        catch { return "error"; }
    }

    [Benchmark]
    public string TryPattern()
    {
        if (int.TryParse("42", out int v)) return v.ToString();
        return "error";
    }

    static Result<int, string> ParseSafe(string s) =>
        int.TryParse(s, out int n)
            ? Result<int, string>.Ok(n)
            : Result<int, string>.Fail($"invalid: {s}");

    static int ParseException(string s) => int.Parse(s);
}
