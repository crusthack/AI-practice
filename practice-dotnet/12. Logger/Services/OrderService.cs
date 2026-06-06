using Microsoft.Extensions.Logging;

namespace _3._Logger.Services;

public class OrderService
{
    private readonly ILogger<OrderService> _logger;

    private static readonly EventId OrderStarted  = new(1001, nameof(OrderStarted));
    private static readonly EventId OrderComplete = new(1002, nameof(OrderComplete));
    private static readonly EventId OrderFailed   = new(1003, nameof(OrderFailed));

    public OrderService(ILogger<OrderService> logger) => _logger = logger;

    public void ProcessOrder(int orderId, string customerId, decimal amount)
    {
        // BeginScope: 이후 모든 로그에 OrderId·CustomerId 컨텍스트 자동 포함
        using var scope = _logger.BeginScope(
            "OrderId={OrderId} CustomerId={CustomerId}",
            orderId,
            customerId);

        _logger.LogInformation(OrderStarted, "주문 처리 시작: {Amount:C}", amount);

        if (amount <= 0)
        {
            _logger.LogError(OrderFailed, "유효하지 않은 금액: {Amount}", amount);
            throw new ArgumentException($"Amount must be positive, was {amount}", nameof(amount));
        }

        if (amount > 1_000_000)
            _logger.LogWarning("고액 주문 감지: {Amount:C} — 추가 검증 필요", amount);

        _logger.LogDebug("주문 유효성 검사 통과");
        _logger.LogInformation(OrderComplete, "주문 처리 완료: {Amount:C}", amount);
    }
}
