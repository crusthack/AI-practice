using Moq;
using Testing.Domain;
using Xunit.Abstractions;

// ── D3: 테스트 패턴 — Fixture · Builder · Output ──────────────────────────────

// xUnit Fixture — 테스트 간 공유 초기화 비용 줄이기
public class SharedProductFixture
{
    public Product Laptop     = new(1, "노트북", 1_500_000m);
    public Product Mouse      = new(2, "마우스", 30_000m);
    public Product Monitor    = new(3, "모니터", 400_000m);
}

public class OrderWithFixtureTests(SharedProductFixture fixture) : IClassFixture<SharedProductFixture>
{
    [Fact] public void MultiLine_TotalIsCorrect()
    {
        var order = new Order { CustomerName = "test" };
        order.AddLine(fixture.Laptop, 1);
        order.AddLine(fixture.Mouse, 2);

        Assert.Equal(1_500_000m + 60_000m, order.Total);
    }

    [Fact] public void Lines_ContainsAll()
    {
        var order = new Order { CustomerName = "test" };
        order.AddLine(fixture.Laptop, 1);
        order.AddLine(fixture.Monitor, 1);
        Assert.Equal(2, order.Lines.Count);
    }
}

// ── Builder Pattern for test data ────────────────────────────────────────────

public class OrderBuilder
{
    private string _customer = "기본고객";
    private readonly List<(Product product, int qty)> _lines = [];

    public OrderBuilder WithCustomer(string name) { _customer = name; return this; }
    public OrderBuilder AddLine(Product p, int qty) { _lines.Add((p, qty)); return this; }

    public Order Build()
    {
        var order = new Order { CustomerName = _customer };
        foreach (var (p, q) in _lines) order.AddLine(p, q);
        return order;
    }
}

public class BuilderPatternTests
{
    private readonly Product _product = new(1, "상품", 100m);

    [Fact] public void Builder_DefaultsWork()
    {
        var order = new OrderBuilder().AddLine(_product, 1).Build();
        Assert.Equal("기본고객", order.CustomerName);
    }

    [Fact] public void Builder_FluentChaining()
    {
        var order = new OrderBuilder()
            .WithCustomer("홍길동")
            .AddLine(_product, 5)
            .AddLine(new Product(2, "다른 상품", 200m), 3)
            .Build();

        Assert.Equal("홍길동", order.CustomerName);
        Assert.Equal(2, order.Lines.Count);
        Assert.Equal(500m + 600m, order.Total);
    }
}

// ── D3: 테스트 출력 / 진단 ────────────────────────────────────────────────────

public class TestOutputTests(ITestOutputHelper output)
{
    [Fact] public void WithOutput_LogsDiagnostics()
    {
        var sw = System.Diagnostics.Stopwatch.StartNew();

        var order = new OrderBuilder()
            .WithCustomer("성능 테스트")
            .AddLine(new Product(1, "item", 1m), 1)
            .Build();

        sw.Stop();
        output.WriteLine($"Order 생성 시간: {sw.ElapsedTicks} ticks");
        output.WriteLine($"Total: {order.Total}");

        Assert.NotNull(order);
    }
}

// ── D3: Assert 다양한 사용 ────────────────────────────────────────────────────

public class AssertVariantsTests
{
    [Fact] public void Assert_Collection_All()
    {
        var orders = new[]
        {
            new OrderBuilder().AddLine(new Product(1, "A", 100m), 1).Build(),
            new OrderBuilder().AddLine(new Product(2, "B", 200m), 1).Build(),
        };
        Assert.All(orders, o => Assert.NotEmpty(o.Lines));
    }

    [Fact] public void Assert_Contains_ByPredicate()
    {
        var order = new OrderBuilder()
            .AddLine(new Product(1, "노트북", 1_000_000m), 1)
            .AddLine(new Product(2, "마우스", 30_000m), 1)
            .Build();

        Assert.Contains(order.Lines, l => l.Product.Name == "노트북");
        Assert.DoesNotContain(order.Lines, l => l.Product.Name == "키보드");
    }

    [Fact] public void Assert_Multiple_ViaRecord()
    {
        // Exception 기록 — 예외 있으면 실패, 없으면 성공
        var ex = Record.Exception(() =>
        {
            var o = new Order { CustomerName = "test" };
            o.AddLine(new Product(1, "item", 100m), 1);
        });
        Assert.Null(ex);
    }

    [Fact] public void Assert_ThrowsAsync_Specific()
    {
        var ex = Assert.Throws<ArgumentException>(() =>
        {
            var order = new Order { CustomerName = "test" };
            order.AddLine(new Product(1, "item", 100m), -1); // 음수 수량
        });
        Assert.Equal("qty", ex.ParamName);
    }

    [Fact] public void Assert_Equivalent_CollectionOrder()
    {
        var list1 = new[] { 3, 1, 2 };
        var list2 = new[] { 1, 2, 3 };
        // 순서 무관 동등성 — xUnit에 내장 없음, 정렬 후 비교
        Assert.Equal(list2.OrderBy(x => x), list1.OrderBy(x => x));
    }
}
