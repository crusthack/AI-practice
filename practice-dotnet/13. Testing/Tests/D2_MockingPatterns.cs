using Moq;
using Testing.Domain;

// ── D2: Moq — Mock · Stub · Fake ──────────────────────────────────────────────

public class OrderServiceMockTests
{
    private readonly Mock<IOrderRepository>  _repoMock  = new();
    private readonly Mock<IEmailService>     _emailMock = new();
    private readonly Mock<IInventoryService> _invMock   = new();
    private OrderService Service => new(_repoMock.Object, _emailMock.Object, _invMock.Object);

    [Fact] public async Task CreateOrder_SavesAndSendsEmail()
    {
        // Arrange
        var product = new Product(1, "노트북", 500_000m);
        _invMock.Setup(i => i.CheckAvailabilityAsync(1, 2)).ReturnsAsync(true);
        _repoMock.Setup(r => r.SaveAsync(It.IsAny<Order>(), default)).Returns(Task.CompletedTask);
        _emailMock.Setup(e => e.SendConfirmationAsync(It.IsAny<string>(), It.IsAny<Order>(), default))
                  .Returns(Task.CompletedTask);

        // Act
        var order = await Service.CreateOrderAsync("홍길동", [(product, 2)]);

        // Assert
        Assert.Equal(OrderStatus.Submitted, order.Status);
        Assert.Equal(1_000_000m, order.Total);
        _repoMock.Verify(r => r.SaveAsync(It.IsAny<Order>(), default), Times.Once);
        _emailMock.Verify(e => e.SendConfirmationAsync("홍길동", It.IsAny<Order>(), default), Times.Once);
    }

    [Fact] public async Task CreateOrder_InsufficientStock_ThrowsAndNoSave()
    {
        var product = new Product(1, "노트북", 500_000m);
        _invMock.Setup(i => i.CheckAvailabilityAsync(1, It.IsAny<int>())).ReturnsAsync(false);

        await Assert.ThrowsAsync<InvalidOperationException>(() =>
            Service.CreateOrderAsync("홍길동", [(product, 2)]));

        _repoMock.Verify(r => r.SaveAsync(It.IsAny<Order>(), default), Times.Never);
        _emailMock.Verify(e => e.SendConfirmationAsync(It.IsAny<string>(), It.IsAny<Order>(), default), Times.Never);
    }

    [Fact] public async Task GetOrder_NotFound_ThrowsKeyNotFound()
    {
        _repoMock.Setup(r => r.FindAsync(99, default)).ReturnsAsync((Order?)null);

        await Assert.ThrowsAsync<KeyNotFoundException>(() =>
            Service.GetOrderAsync(99));
    }

    [Fact] public async Task CreateOrder_EmptyCustomer_Throws()
    {
        await Assert.ThrowsAsync<ArgumentException>(() =>
            Service.CreateOrderAsync("", []));
    }
}

// ── D2: Mock 고급 — Callback · Sequence · Protected ──────────────────────────

public class MockAdvancedTests
{
    [Fact] public async Task Mock_Callback_CapturesArguments()
    {
        var mock = new Mock<IOrderRepository>();
        Order? saved = null;

        mock.Setup(r => r.SaveAsync(It.IsAny<Order>(), default))
            .Callback<Order, CancellationToken>((o, _) => saved = o)
            .Returns(Task.CompletedTask);

        var order = new Order { CustomerName = "test" };
        order.AddLine(new Product(1, "item", 100m), 1);
        await mock.Object.SaveAsync(order, default);

        Assert.NotNull(saved);
        Assert.Equal("test", saved!.CustomerName);
    }

    [Fact] public async Task Mock_SetupSequence_DifferentReturns()
    {
        var mock = new Mock<IInventoryService>();
        mock.SetupSequence(i => i.CheckAvailabilityAsync(1, 1))
            .ReturnsAsync(true)   // 첫 번째 호출
            .ReturnsAsync(false); // 두 번째 호출

        Assert.True(await mock.Object.CheckAvailabilityAsync(1, 1));
        Assert.False(await mock.Object.CheckAvailabilityAsync(1, 1));
    }

    [Fact] public async Task Mock_Throws_WhenExpected()
    {
        var mock = new Mock<IEmailService>();
        mock.Setup(e => e.SendConfirmationAsync(It.IsAny<string>(), It.IsAny<Order>(), default))
            .ThrowsAsync(new TimeoutException("메일 서버 타임아웃"));

        await Assert.ThrowsAsync<TimeoutException>(() =>
            mock.Object.SendConfirmationAsync("user@test.com", null!, default));
    }

    [Fact] public async Task Mock_ItIsAny_MatchesAll()
    {
        var mock = new Mock<IInventoryService>();
        mock.Setup(i => i.CheckAvailabilityAsync(It.IsAny<int>(), It.IsAny<int>()))
            .ReturnsAsync(true);

        Assert.True(await mock.Object.CheckAvailabilityAsync(1, 1));
        Assert.True(await mock.Object.CheckAvailabilityAsync(999, 100));
        mock.Verify(i => i.CheckAvailabilityAsync(It.IsAny<int>(), It.IsAny<int>()), Times.Exactly(2));
    }
}

// ── D2: Fake — 실제 구현 교체 ─────────────────────────────────────────────────

public class FakeOrderRepository : IOrderRepository
{
    private readonly Dictionary<int, Order> _store = [];

    public Task<Order?> FindAsync(int id, CancellationToken ct = default) =>
        Task.FromResult(_store.GetValueOrDefault(id));

    public Task SaveAsync(Order order, CancellationToken ct = default)
    {
        _store[order.Id] = order;
        return Task.CompletedTask;
    }

    public Task<IEnumerable<Order>> GetByCustomerAsync(string name, CancellationToken ct = default) =>
        Task.FromResult(_store.Values.Where(o => o.CustomerName == name));
}

public class FakeTests
{
    [Fact] public async Task FakeRepo_StoresAndRetrieves()
    {
        var repo = new FakeOrderRepository();
        var order = new Order { Id = 1, CustomerName = "test" };
        order.AddLine(new Product(1, "item", 100m), 1);

        await repo.SaveAsync(order);
        var found = await repo.FindAsync(1);

        Assert.NotNull(found);
        Assert.Equal("test", found!.CustomerName);
    }
}
