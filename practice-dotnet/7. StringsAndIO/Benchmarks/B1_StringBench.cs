using System.Text;
using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Running;

BenchmarkRunner.Run<StringBench>();

[MemoryDiagnoser, ShortRunJob]
public class StringBench
{
    private const int N = 1000;
    private static readonly string[] Parts = Enumerable.Range(0, N).Select(i => i.ToString()).ToArray();

    [Benchmark(Baseline = true)]
    public string StringConcatLoop()
    {
        string result = "";
        foreach (var p in Parts) result += p + ",";
        return result;
    }

    [Benchmark]
    public string StringBuilderLoop()
    {
        var sb = new StringBuilder();
        foreach (var p in Parts) sb.Append(p).Append(',');
        return sb.ToString();
    }

    [Benchmark]
    public string StringJoin() => string.Join(",", Parts);
}
