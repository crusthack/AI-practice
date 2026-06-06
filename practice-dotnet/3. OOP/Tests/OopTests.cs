using OOP.Demos;

// ── 상속 ─────────────────────────────────────────────────────────────────

public class InheritanceTests
{
    [Fact] public void Dog_Speak() => Assert.Equal("멍멍!", new Dog("x").Speak());
    [Fact] public void Cat_Speak() => Assert.Equal("야옹~", new Cat("x").Speak());
    [Fact] public void Dog_Describe_Contains_Base()
        => Assert.Contains("이름=", new Dog("뭉치").Describe());

    [Fact]
    public void Override_Via_Base_Ref()
    {
        Base b = new OverrideChild();
        Assert.Equal("OverrideChild", b.Greet());
    }

    [Fact]
    public void Hiding_Via_Base_Ref()
    {
        Base b = new HidingChild();
        Assert.Equal("Base", b.Greet());   // hiding: 부모 메서드 실행
    }

    [Fact]
    public void Covariant_Return_Is_Dog()
    {
        Dog d = new Dog("원");
        Assert.IsType<Dog>(d.Clone());
    }

    [Fact]
    public void Constructor_Chain_Order()
    {
        var log = new List<string>();
        _ = new Car("현대", "소나타", log);
        Assert.Equal(2, log.Count);
        Assert.Contains("[Vehicle]", log[0]);
        Assert.Contains("[Car]", log[1]);
    }
}

// ── 다형성 ───────────────────────────────────────────────────────────────

public class PolymorphismTests
{
    [Fact]
    public void Vector2D_Add()
    {
        var r = new Vector2D(1, 2) + new Vector2D(3, 4);
        Assert.Equal(new Vector2D(4, 6), r);
    }

    [Fact]
    public void Vector2D_ScalarMul()
    {
        var r = new Vector2D(3, 4) * 2;
        Assert.Equal(new Vector2D(6, 8), r);
    }

    [Fact]
    public void Vector2D_Equality()
    {
        Assert.True(new Vector2D(1, 1) == new Vector2D(1, 1));
        Assert.True(new Vector2D(1, 1) != new Vector2D(1, 2));
    }

    [Fact]
    public void Celsius_To_Fahrenheit()
    {
        Fahrenheit f = (Fahrenheit)new Celsius(100);
        Assert.Equal(212, f.Value, precision: 5);
    }

    [Fact]
    public void Matrix_Indexer()
    {
        var m = new Matrix(2, 2);
        m[0, 1] = 42;
        Assert.Equal(42, m[0, 1]);
        Assert.Equal(0,  m[1, 0]);
    }

    [Fact]
    public void Config_Indexer_DefaultValue()
    {
        var c = new Config();
        c["key"] = "val";
        Assert.Equal("val",    c["key"]);
        Assert.Equal("(기본값)", c["none"]);
    }
}

// ── 제네릭 ────────────────────────────────────────────────────────────────

public class GenericsTests
{
    [Fact]
    public void Repository_CRUD()
    {
        var repo = new Repository<Product2, int>();
        repo.Add(new Product2(1, "A", 100m));
        repo.Add(new Product2(2, "B", 200m));

        Assert.Equal(2, repo.Count);
        Assert.Equal("A", repo.Get(1)!.Name);

        repo.Update(new Product2(1, "A+", 150m));
        Assert.Equal("A+", repo.Get(1)!.Name);

        repo.Delete(2);
        Assert.Equal(1, repo.Count);
        Assert.Null(repo.Get(2));
    }

    [Fact]
    public void GenericMath_Sum()
    {
        Assert.Equal(15,   GenericMath.Sum(1, 2, 3, 4, 5));
        Assert.Equal(15.0, GenericMath.Sum(1.0, 2.0, 3.0, 4.0, 5.0));
    }

    [Fact]
    public void GenericMath_Clamp()
    {
        Assert.Equal(10, GenericMath.Clamp(150, 0, 10));
        Assert.Equal(0,  GenericMath.Clamp(-5, 0, 10));
        Assert.Equal(5,  GenericMath.Clamp(5, 0, 10));
    }
}

// ── 이벤트 ───────────────────────────────────────────────────────────────

public class EventTests
{
    [Fact]
    public void Event_Fires_On_Price_Change()
    {
        var store = new StockStore();
        string? received = null;
        store.PriceChanged += (_, e) => received = e.Ticker;

        store.UpdatePrice("삼성전자", 70_000);
        Assert.Equal("삼성전자", received);
    }

    [Fact]
    public void Event_Unsubscribe_Stops_Firing()
    {
        var store = new StockStore();
        int count = 0;
        EventHandler<PriceEventArgs> handler = (_, _) => count++;

        store.PriceChanged += handler;
        store.UpdatePrice("A", 100);
        store.PriceChanged -= handler;
        store.UpdatePrice("A", 200);

        Assert.Equal(1, count);
    }
}

// ── 합성 ─────────────────────────────────────────────────────────────────

public class CompositionTests
{
    [Fact]
    public void Decorator_Stacks_Cost()
    {
        ICoffee coffee = new WhipDecorator(new MilkDecorator(new SimpleCoffee()));
        Assert.Equal(1_500m + 500m + 800m, coffee.Cost());
        Assert.Contains("우유", coffee.Description);
        Assert.Contains("휘핑크림", coffee.Description);
    }

    [Fact]
    public void Strategy_BubbleSort()
    {
        var s = new Sorter<int> { Strategy = new BubbleSortStrategy<int>() };
        int[] arr = [3, 1, 2];
        s.Sort(arr);
        Assert.Equal(new[] { 1, 2, 3 }, arr);
    }

    [Fact]
    public void Extension_IsStrongPassword()
    {
        Assert.False("password".IsStrongPassword());
        Assert.True("P@ssw0rd!".IsStrongPassword());
    }

    [Fact]
    public void Extension_Batch()
    {
        var batches = new[] { 1, 2, 3, 4, 5 }.Batch(2).ToList();
        Assert.Equal(3, batches.Count);
        Assert.Equal(new[] { 1, 2 }, batches[0]);
        Assert.Equal(new[] { 5 },   batches[2]);
    }

    [Fact]
    public void Disposable_DoWork_Throws_After_Dispose()
    {
        var r = new ManagedResource("test");
        r.Dispose();
        Assert.Throws<ObjectDisposedException>(() => r.DoWork());
    }
}

public class AdvancedPatternTests
{
    [Fact]
    public void OrderFactory_CreatesDraftAndComputesTotal()
    {
        var order = OrderFactory.CreateDraft("C-001")
            .AddLine("A", 100m, 2)
            .AddLine("B", 50m, 1);

        Assert.Equal("C-001", order.CustomerId);
        Assert.Equal(250m, order.Total);
        Assert.Equal(2, order.Lines.Count);
    }

    [Fact]
    public void Specification_And_ComposesRules()
    {
        var order = OrderFactory.CreateDraft("C-001")
            .AddLine("Mouse", 35_000m, 5);

        var spec = new HighValueOrderSpec(100_000m)
            .And(new ContainsProductSpec("mouse"));

        Assert.True(spec.IsSatisfiedBy(order));
    }

    [Fact]
    public void Visitor_AccumulatesArea()
    {
        ShapeNode[] shapes =
        [
            new CircleNode(1),
            new RectangleNode(2, 3)
        ];

        var visitor = new AreaVisitor();
        foreach (var shape in shapes)
            shape.Accept(visitor);

        Assert.Equal(Math.PI + 6, visitor.TotalArea, precision: 10);
    }

    [Fact]
    public async Task AsyncSession_ThrowsAfterDispose()
    {
        var session = new AsyncSession("test");
        await session.DisposeAsync();

        await Assert.ThrowsAsync<ObjectDisposedException>(async () =>
            await session.SendAsync("after dispose"));
    }
}
