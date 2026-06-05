using _2._HostBuilder.Options;
using Microsoft.Extensions.Options;

namespace _2._HostBuilder.Services;

public class GreetingService : IGreetingService
{
    private readonly AppSettings _settings;

    public GreetingService(IOptions<AppSettings> options)
        => _settings = options.Value;

    public string Greet(string name) =>
        $"[{_settings.Name}] Hello, {name}! (최대 재시도: {_settings.MaxRetries})";
}
