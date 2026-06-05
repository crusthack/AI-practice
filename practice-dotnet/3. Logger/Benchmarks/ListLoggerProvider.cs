using Microsoft.Extensions.Logging;

// 메모리에 로그를 기록하는 커스텀 프로바이더
// I/O 없이 로깅 인프라 자체의 오버헤드만 측정하기 위해 사용
public sealed class ListLoggerProvider : ILoggerProvider
{
    public void Dispose() { }
    public ILogger CreateLogger(string categoryName) => new ListLogger();
}

public sealed class ListLogger : ILogger
{
    // static으로 공유: 여러 인스턴스가 같은 버킷에 기록 (GC 폭주 방지)
    private static readonly List<string> Sink = [];

    public IDisposable? BeginScope<TState>(TState state) where TState : notnull
        => ScopeDisposable.Instance;

    public bool IsEnabled(LogLevel logLevel) => logLevel != LogLevel.None;

    public void Log<TState>(LogLevel logLevel, EventId eventId, TState state,
        Exception? exception, Func<TState, Exception?, string> formatter)
    {
        var msg = formatter(state, exception); // 포매팅 비용은 지불
        lock (Sink)
        {
            Sink.Add(msg);
            if (Sink.Count > 50_000) Sink.Clear(); // 메모리 한도
        }
    }

    private sealed class ScopeDisposable : IDisposable
    {
        public static readonly ScopeDisposable Instance = new();
        public void Dispose() { }
    }
}
