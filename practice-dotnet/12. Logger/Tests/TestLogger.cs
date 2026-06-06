using Microsoft.Extensions.Logging;

namespace _3._Logger.Tests;

public record LogEntry(
    LogLevel Level,
    string Message,
    Exception? Exception,
    EventId EventId,
    IReadOnlyList<object> Scopes);

// 테스트용 ILogger<T> 구현: 로그 엔트리를 메모리에 수집
public class TestLogger<T> : ILogger<T>
{
    private readonly List<LogEntry> _entries = [];
    private readonly List<object> _scopes = [];
    public IReadOnlyList<LogEntry> Entries => _entries;

    public IDisposable? BeginScope<TState>(TState state) where TState : notnull
    {
        _scopes.Add(state);
        return new Scope(() => _scopes.Remove(state));
    }

    public bool IsEnabled(LogLevel logLevel) => logLevel != LogLevel.None;

    public void Log<TState>(LogLevel logLevel, EventId eventId, TState state,
        Exception? exception, Func<TState, Exception?, string> formatter)
    {
        _entries.Add(new LogEntry(
            logLevel,
            formatter(state, exception),
            exception,
            eventId,
            _scopes.ToArray()));
    }

    private sealed class Scope(Action dispose) : IDisposable
    {
        public void Dispose() => dispose();
    }
}
