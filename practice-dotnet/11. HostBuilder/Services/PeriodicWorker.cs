using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;

namespace _2._HostBuilder.Services;

// BackgroundService: IHostedService의 추상 기반 클래스.
// ExecuteAsync가 호스트 수명 동안 백그라운드에서 계속 실행됨.
public class PeriodicWorker : BackgroundService
{
    private readonly ILogger<PeriodicWorker> _logger;
    private readonly CounterService _counter;

    public PeriodicWorker(ILogger<PeriodicWorker> logger, CounterService counter)
    {
        _logger  = logger;
        _counter = counter;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        _logger.LogInformation("PeriodicWorker 시작");

        while (!stoppingToken.IsCancellationRequested)
        {
            int tick = _counter.Increment();
            _logger.LogInformation("Tick #{Tick} @ {Time:HH:mm:ss}",
                tick, DateTimeOffset.Now);

            try
            {
                await Task.Delay(TimeSpan.FromSeconds(2), stoppingToken);
            }
            catch (OperationCanceledException)
            {
                break;
            }
        }

        _logger.LogInformation("PeriodicWorker 종료 (총 틱: {Total})", _counter.Current);
    }
}
