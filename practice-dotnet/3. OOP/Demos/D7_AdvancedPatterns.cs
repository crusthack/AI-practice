namespace OOP.Demos;

static class D7_AdvancedPatterns
{
    public static async Task RunAsync()
    {
        Print.Header("7. Advanced OOP Patterns");

        ShowFactoryAndSpecification();
        ShowVisitor();
        await ShowAsyncDisposableAsync();
    }

    static void ShowFactoryAndSpecification()
    {
        Print.Section("7-1. Factory + Specification");

        var order = OrderFactory.CreateDraft("C-001")
            .AddLine("Keyboard", 120_000m, 1)
            .AddLine("Mouse", 35_000m, 2);

        ISpecification<SalesOrder> highValue = new HighValueOrderSpec(150_000m);
        ISpecification<SalesOrder> hasMouse = new ContainsProductSpec("Mouse");
        ISpecification<SalesOrder> combined = highValue.And(hasMouse);

        Print.Line($"order total: {order.Total:C}");
        Print.Line($"high value + mouse: {combined.IsSatisfiedBy(order)}");
    }

    static void ShowVisitor()
    {
        Print.Section("7-2. Visitor - add operations without changing shape types");

        ShapeNode[] shapes =
        [
            new CircleNode(3),
            new RectangleNode(4, 5)
        ];

        var areaVisitor = new AreaVisitor();
        foreach (ShapeNode shape in shapes)
            shape.Accept(areaVisitor);

        Print.Line($"total area: {areaVisitor.TotalArea:F2}");
    }

    static async Task ShowAsyncDisposableAsync()
    {
        Print.Section("7-3. IAsyncDisposable - async resource cleanup");

        await using var resource = new AsyncSession("session-1");
        await resource.SendAsync("hello");
        Print.Line($"sent count: {resource.SentCount}");
    }
}

public sealed class SalesOrder
{
    private readonly List<OrderLine> _lines = [];

    internal SalesOrder(string customerId) => CustomerId = customerId;

    public string CustomerId { get; }
    public IReadOnlyList<OrderLine> Lines => _lines;
    public decimal Total => _lines.Sum(line => line.Subtotal);

    public SalesOrder AddLine(string product, decimal unitPrice, int quantity)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(product);
        ArgumentOutOfRangeException.ThrowIfNegative(unitPrice);
        ArgumentOutOfRangeException.ThrowIfLessThan(quantity, 1);

        _lines.Add(new OrderLine(product, unitPrice, quantity));
        return this;
    }
}

public sealed record OrderLine(string Product, decimal UnitPrice, int Quantity)
{
    public decimal Subtotal => UnitPrice * Quantity;
}

public static class OrderFactory
{
    public static SalesOrder CreateDraft(string customerId)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(customerId);
        return new SalesOrder(customerId);
    }
}

public interface ISpecification<in T>
{
    bool IsSatisfiedBy(T candidate);
}

public static class SpecificationExtensions
{
    public static ISpecification<T> And<T>(
        this ISpecification<T> left,
        ISpecification<T> right)
    {
        ArgumentNullException.ThrowIfNull(left);
        ArgumentNullException.ThrowIfNull(right);
        return new AndSpecification<T>(left, right);
    }
}

sealed class AndSpecification<T>(
    ISpecification<T> left,
    ISpecification<T> right) : ISpecification<T>
{
    public bool IsSatisfiedBy(T candidate) =>
        left.IsSatisfiedBy(candidate) && right.IsSatisfiedBy(candidate);
}

public sealed class HighValueOrderSpec(decimal minimumTotal) : ISpecification<SalesOrder>
{
    public bool IsSatisfiedBy(SalesOrder candidate) => candidate.Total >= minimumTotal;
}

public sealed class ContainsProductSpec(string product) : ISpecification<SalesOrder>
{
    public bool IsSatisfiedBy(SalesOrder candidate) =>
        candidate.Lines.Any(line => line.Product.Equals(product, StringComparison.OrdinalIgnoreCase));
}

public abstract class ShapeNode
{
    public abstract void Accept(IShapeVisitor visitor);
}

public sealed class CircleNode(double radius) : ShapeNode
{
    public double Radius { get; } = radius;
    public override void Accept(IShapeVisitor visitor) => visitor.Visit(this);
}

public sealed class RectangleNode(double width, double height) : ShapeNode
{
    public double Width { get; } = width;
    public double Height { get; } = height;
    public override void Accept(IShapeVisitor visitor) => visitor.Visit(this);
}

public interface IShapeVisitor
{
    void Visit(CircleNode circle);
    void Visit(RectangleNode rectangle);
}

public sealed class AreaVisitor : IShapeVisitor
{
    public double TotalArea { get; private set; }

    public void Visit(CircleNode circle) =>
        TotalArea += Math.PI * circle.Radius * circle.Radius;

    public void Visit(RectangleNode rectangle) =>
        TotalArea += rectangle.Width * rectangle.Height;
}

public sealed class AsyncSession(string name) : IAsyncDisposable
{
    private bool _disposed;

    public int SentCount { get; private set; }

    public async ValueTask SendAsync(string message)
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        ArgumentException.ThrowIfNullOrWhiteSpace(message);

        await Task.Delay(1);
        SentCount++;
        Print.Line($"{name}: sent \"{message}\"");
    }

    public async ValueTask DisposeAsync()
    {
        if (_disposed)
            return;

        await Task.Delay(1);
        _disposed = true;
        Print.Line($"{name}: async cleanup complete");
    }
}
