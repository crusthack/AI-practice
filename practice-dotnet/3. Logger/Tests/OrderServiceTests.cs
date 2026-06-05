using _3._Logger.Services;
using Microsoft.Extensions.Logging;

namespace _3._Logger.Tests;

public class OrderServiceTests
{
    [Fact]
    public void ProcessOrder_ValidOrder_LogsStartAndComplete()
    {
        var logger  = new TestLogger<OrderService>();
        var service = new OrderService(logger);

        service.ProcessOrder(1001, "C-001", 250m);

        Assert.Contains(logger.Entries, e =>
            e.Level == LogLevel.Information && e.Message.Contains("시작"));
        Assert.Contains(logger.Entries, e =>
            e.Level == LogLevel.Information && e.Message.Contains("완료"));
        Assert.DoesNotContain(logger.Entries, e => e.Level >= LogLevel.Error);
    }

    [Fact]
    public void ProcessOrder_ZeroAmount_LogsErrorAndThrows()
    {
        var logger  = new TestLogger<OrderService>();
        var service = new OrderService(logger);

        Assert.Throws<ArgumentException>(() => service.ProcessOrder(1, "C-001", 0m));
        Assert.Contains(logger.Entries, e => e.Level == LogLevel.Error);
    }

    [Fact]
    public void ProcessOrder_HighAmount_LogsWarning()
    {
        var logger  = new TestLogger<OrderService>();
        var service = new OrderService(logger);

        service.ProcessOrder(1, "C-001", 1_500_000m);

        Assert.Contains(logger.Entries, e => e.Level == LogLevel.Warning);
    }

    [Fact]
    public void ProcessOrder_NormalAmount_NoWarning()
    {
        var logger  = new TestLogger<OrderService>();
        var service = new OrderService(logger);

        service.ProcessOrder(1, "C-001", 100m);

        Assert.DoesNotContain(logger.Entries, e => e.Level == LogLevel.Warning);
    }
}
