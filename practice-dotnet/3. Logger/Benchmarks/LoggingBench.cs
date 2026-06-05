using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;

// ─────────────────────────────────────────────────────
// 로깅 전략별 성능 비교
//
// NullLogger    : 완전 no-op. 절대 하한선.
// ListLogger    : 메모리에 기록. I/O 없이 인프라 오버헤드만 측정.
// FilteredOut   : IsEnabled=false → 포매팅 자체가 생략됨.
// IsEnabled 가드: 비용이 큰 문자열 보간을 조건부로 실행.
// BeginScope    : 스코프 생성·소멸 비용.
// ─────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class LoggingBench
{
    private ILogger<LoggingBench> _null      = null!;
    private ILogger<LoggingBench> _list      = null!;
    private ILogger<LoggingBench> _filtered  = null!;

    [GlobalSetup]
    public void Setup()
    {
        _null = NullLogger<LoggingBench>.Instance;

        _list = LoggerFactory.Create(b =>
            b.AddProvider(new ListLoggerProvider())
             .SetMinimumLevel(LogLevel.Debug))
            .CreateLogger<LoggingBench>();

        _filtered = LoggerFactory.Create(b =>
            b.AddProvider(new ListLoggerProvider())
             .SetMinimumLevel(LogLevel.Warning))
            .CreateLogger<LoggingBench>();
    }

    // 완전 no-op: 절대 하한선
    [Benchmark(Baseline = true)]
    public void NullLogger_Info() =>
        _null.LogInformation("Order {OrderId} processed in {Elapsed}ms", 42, 100);

    // 포매팅 + 메모리 기록 비용
    [Benchmark]
    public void ListLogger_Info() =>
        _list.LogInformation("Order {OrderId} processed in {Elapsed}ms", 42, 100);

    // IsEnabled=false → 포매팅 완전 생략
    [Benchmark]
    public void FilteredOut_Debug() =>
        _filtered.LogDebug("Verbose detail: {Data}", new object());

    // IsEnabled 가드로 불필요한 보간 방지
    [Benchmark]
    public void WithIsEnabledGuard()
    {
        if (_list.IsEnabled(LogLevel.Debug))
            _list.LogDebug("Expensive: {Data}", string.Join(",", Enumerable.Range(0, 100)));
    }

    // BeginScope 생성·소멸 오버헤드
    [Benchmark]
    public void WithScope()
    {
        using var scope = _list.BeginScope("OrderId={OrderId}", 42);
        _list.LogInformation("처리 중");
    }
}
