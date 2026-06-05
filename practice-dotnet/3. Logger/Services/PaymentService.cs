using Microsoft.Extensions.Logging;

namespace _3._Logger.Services;

public class PaymentService
{
    private readonly ILogger<PaymentService> _logger;

    public PaymentService(ILogger<PaymentService> logger) => _logger = logger;

    public async Task<bool> ProcessPaymentAsync(
        int orderId, decimal amount, CancellationToken ct = default)
    {
        using var scope = _logger.BeginScope("Payment[Order={OrderId}]", orderId);

        _logger.LogDebug("결제 게이트웨이 호출 시작: {Amount:C}", amount);

        try
        {
            await Task.Delay(TimeSpan.FromMilliseconds(50), ct);
            _logger.LogInformation("결제 성공: {Amount:C}", amount);
            return true;
        }
        catch (OperationCanceledException)
        {
            _logger.LogWarning("결제 취소됨 (CancellationToken)");
            return false;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "결제 처리 중 예외 발생");
            throw;
        }
    }
}
