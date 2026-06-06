namespace Testing.Domain;

// 테스트 대상 도메인 모델 / 서비스

public record Product(int Id, string Name, decimal Price);
public record OrderLine(Product Product, int Quantity)
{
    public decimal Total => Product.Price * Quantity;
}

public class Order
{
    private readonly List<OrderLine> _lines = [];
    public int Id { get; init; }
    public string CustomerName { get; init; } = "";
    public IReadOnlyList<OrderLine> Lines => _lines.AsReadOnly();
    public decimal Total => _lines.Sum(l => l.Total);
    public OrderStatus Status { get; private set; } = OrderStatus.Pending;

    public void AddLine(Product product, int qty)
    {
        if (qty <= 0)        throw new ArgumentException("수량은 양수여야 합니다.", nameof(qty));
        if (product is null) throw new ArgumentNullException(nameof(product));
        _lines.Add(new OrderLine(product, qty));
    }

    public void Submit()
    {
        if (_lines.Count == 0) throw new InvalidOperationException("빈 주문은 제출할 수 없습니다.");
        if (Status != OrderStatus.Pending) throw new InvalidOperationException($"상태 오류: {Status}");
        Status = OrderStatus.Submitted;
    }

    public void Cancel()
    {
        if (Status == OrderStatus.Shipped) throw new InvalidOperationException("배송 후 취소 불가");
        Status = OrderStatus.Canceled;
    }
}

public enum OrderStatus { Pending, Submitted, Shipped, Canceled }

// ── 인터페이스 / 서비스 ─────────────────────────────────────────────────────────

public interface IOrderRepository
{
    Task<Order?> FindAsync(int id, CancellationToken ct = default);
    Task SaveAsync(Order order, CancellationToken ct = default);
    Task<IEnumerable<Order>> GetByCustomerAsync(string name, CancellationToken ct = default);
}

public interface IEmailService
{
    Task SendConfirmationAsync(string to, Order order, CancellationToken ct = default);
}

public interface IInventoryService
{
    Task<bool> CheckAvailabilityAsync(int productId, int qty);
}

public class OrderService(
    IOrderRepository repo,
    IEmailService emailService,
    IInventoryService inventory)
{
    public async Task<Order> CreateOrderAsync(
        string customer,
        List<(Product product, int qty)> items,
        CancellationToken ct = default)
    {
        if (string.IsNullOrWhiteSpace(customer))
            throw new ArgumentException("고객명 필수", nameof(customer));

        foreach (var (product, qty) in items)
        {
            bool available = await inventory.CheckAvailabilityAsync(product.Id, qty);
            if (!available)
                throw new InvalidOperationException($"재고 부족: {product.Name}");
        }

        var order = new Order { Id = Random.Shared.Next(1, 9999), CustomerName = customer };
        foreach (var (product, qty) in items)
            order.AddLine(product, qty);

        order.Submit();
        await repo.SaveAsync(order, ct);
        await emailService.SendConfirmationAsync(customer, order, ct);
        return order;
    }

    public async Task<Order> GetOrderAsync(int id, CancellationToken ct = default) =>
        await repo.FindAsync(id, ct) ?? throw new KeyNotFoundException($"Order {id} not found");
}

// ── 계산 유틸 ──────────────────────────────────────────────────────────────────

public static class PricingEngine
{
    public static decimal ApplyDiscount(decimal price, int qty, DiscountPolicy policy) =>
        policy switch
        {
            DiscountPolicy.None       => price * qty,
            DiscountPolicy.Bulk10     => price * qty * (qty >= 10 ? 0.9m : 1m),
            DiscountPolicy.Graduated  => price * qty * GraduatedRate(qty),
            _                         => throw new ArgumentOutOfRangeException(nameof(policy))
        };

    private static decimal GraduatedRate(int qty) =>
        qty switch { >= 100 => 0.7m, >= 50 => 0.8m, >= 10 => 0.9m, _ => 1m };
}

public enum DiscountPolicy { None, Bulk10, Graduated }
