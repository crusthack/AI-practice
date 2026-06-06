using TypeSystem;

namespace TypeSystem.Tests;

// ── 값 타입 테스트 ────────────────────────────────────────────────────────────
public class ValueTypeTests
{
    [Fact]
    public void MutablePoint_CopySemantics()
    {
        var a = new MutablePoint(1, 2);
        var b = a;
        b.X = 99;

        Assert.Equal(1, a.X);   // 복사본 변경이 원본에 영향 없어야 함
        Assert.Equal(99, b.X);
    }

    [Fact]
    public void MutablePoint_Equality()
    {
        var a = new MutablePoint(3, 4);
        var b = new MutablePoint(3, 4);
        var c = new MutablePoint(0, 0);

        Assert.True(a == b);
        Assert.False(a == c);
        Assert.True(a.Equals(b));
        Assert.Equal(a.GetHashCode(), b.GetHashCode());
    }

    [Fact]
    public void ImmutablePoint_TranslateCreatesNewInstance()
    {
        var p     = new ImmutablePoint(1, 2);
        var moved = p.Translate(3, 4);

        Assert.Equal(1, p.X);      // 원본 불변
        Assert.Equal(2, p.Y);
        Assert.Equal(4, moved.X);
        Assert.Equal(6, moved.Y);
    }

    [Fact]
    public void ImmutablePoint_Distance()
    {
        var p = new ImmutablePoint(3, 4);
        Assert.Equal(5.0, p.Distance, precision: 10);
    }

    [Fact]
    public void RecordPoint_ValueEquality()
    {
        var r1 = new RecordPoint(1, 2);
        var r2 = new RecordPoint(1, 2);
        var r3 = new RecordPoint(9, 9);

        Assert.Equal(r1, r2);
        Assert.NotEqual(r1, r3);
    }

    [Fact]
    public void RecordPoint_WithExpression()
    {
        var r1 = new RecordPoint(1, 2);
        var r2 = r1 with { Y = 99 };

        Assert.Equal(2, r1.Y);    // 원본 유지: r1.Y는 초기값 2 그대로
        Assert.Equal(99, r2.Y);
    }

    [Fact]
    public void Temperature_Conversions()
    {
        var boiling = new Temperature(100);

        Assert.Equal(212.0, boiling.Fahrenheit, precision: 10);
        Assert.Equal(373.15, boiling.Kelvin, precision: 10);

        var fromF = Temperature.FromFahrenheit(32);
        Assert.Equal(0.0, fromF.Celsius, precision: 10);
    }

    [Fact]
    public void Temperature_Add_CreatesNewInstance()
    {
        var t1 = new Temperature(20);
        var t2 = t1.Add(5);

        Assert.Equal(20, t1.Celsius);  // 불변
        Assert.Equal(25, t2.Celsius);
    }

    [Fact]
    public void SpanWrapper_SumAndIndexAccess()
    {
        Span<int> data = stackalloc int[] { 1, 2, 3, 4, 5 };
        var wrapper = new SpanWrapper(data);

        Assert.Equal(5, wrapper.Length);
        Assert.Equal(15, wrapper.Sum());

        wrapper[0] = 10;
        Assert.Equal(24, wrapper.Sum());
    }

    [Fact]
    public void FilePermission_FlagsComposition()
    {
        var perm = FilePermission.Read | FilePermission.Write;

        Assert.True(perm.HasFlag(FilePermission.Read));
        Assert.True(perm.HasFlag(FilePermission.Write));
        Assert.False(perm.HasFlag(FilePermission.Execute));
        Assert.Equal(FilePermission.ReadWrite, perm);
    }
}

// ── 참조 타입 테스트 ──────────────────────────────────────────────────────────
public class ReferenceTypeTests
{
    [Fact]
    public void PersonClass_ReferenceSemantics()
    {
        var a = new PersonClass("Alice", 30);
        var b = a;   // 참조 복사
        b.Age = 99;

        Assert.Equal(99, a.Age);   // 같은 객체를 가리키므로 둘 다 변경
        Assert.True(ReferenceEquals(a, b));
    }

    [Fact]
    public void PersonClass_DefaultEquality_IsReference()
    {
        var a = new PersonClass("Alice", 30);
        var b = new PersonClass("Alice", 30);

        Assert.False(a.Equals(b));  // 기본 Equals = 참조 비교
        Assert.False(ReferenceEquals(a, b));
    }

    [Fact]
    public void ServerConfig_InitOnlyAfterCreation()
    {
        var cfg = new ServerConfig { Host = "localhost", Port = 80 };

        Assert.Equal("localhost", cfg.Host);
        Assert.Equal(80, cfg.Port);
        Assert.True(cfg.UseTls);          // 기본값
        Assert.Equal(30_000, cfg.TimeoutMs);
    }

    [Fact]
    public void ApiToken_Validity()
    {
        Assert.True(new ApiToken("abcdef1234567890").IsValid);
        Assert.False(new ApiToken("short").IsValid);
        Assert.False(new ApiToken("").IsValid);
    }

    [Fact]
    public void Circle_AreaAndPerimeter()
    {
        var c = new Circle(5);

        Assert.Equal(Math.PI * 25, c.Area, precision: 10);
        Assert.Equal(Math.PI * 10, c.Perimeter, precision: 10);
    }

    [Fact]
    public void Rectangle_AreaAndPerimeter()
    {
        var r = new Rectangle(4, 6);

        Assert.Equal(24, r.Area, precision: 10);
        Assert.Equal(20, r.Perimeter, precision: 10);
    }

    [Fact]
    public void Result_Ok_Map()
    {
        var result = Result<int>.Ok(5).Map(v => v * v);

        Assert.True(result.IsSuccess);
        Assert.Equal(25, result.Value);
    }

    [Fact]
    public void Result_Fail_PropagatesError()
    {
        var result = Result<int>.Fail("error").Map(v => v * v);

        Assert.False(result.IsSuccess);
        Assert.Equal("error", result.Error);
    }

    [Fact]
    public void Order_TotalCalculation()
    {
        var order = new Order { Id = 1 };
        order.Items.Add(new Order.LineItem("A", 100m, 3));
        order.Items.Add(new Order.LineItem("B", 200m, 2));

        Assert.Equal(700m, order.Total);
    }
}

// ── Record 타입 테스트 ────────────────────────────────────────────────────────
public class RecordTypeTests
{
    [Fact]
    public void PersonRecord_ValueEquality()
    {
        var p1 = new PersonRecord("Alice", 30);
        var p2 = new PersonRecord("Alice", 30);

        Assert.Equal(p1, p2);
        Assert.True(p1 == p2);
        Assert.False(ReferenceEquals(p1, p2));
    }

    [Fact]
    public void PersonRecord_With_CreatesNewInstance()
    {
        var p1 = new PersonRecord("Alice", 30);
        var p2 = p1 with { Age = 31 };

        Assert.Equal(30, p1.Age);  // 원본 불변
        Assert.Equal(31, p2.Age);
        Assert.NotEqual(p1, p2);
    }

    [Fact]
    public void PersonRecord_Deconstruct()
    {
        var p = new PersonRecord("Bob", 25);
        var (name, age) = p;

        Assert.Equal("Bob", name);
        Assert.Equal(25, age);
    }

    [Fact]
    public void Product_StaticFactory_Validates()
    {
        Assert.Throws<ArgumentException>(() => Product.Create("", "name", 100m));
        Assert.Throws<ArgumentOutOfRangeException>(() => Product.Create("P1", "name", -1m));
    }

    [Fact]
    public void Product_WithDiscount()
    {
        var p = Product.Create("P1", "item", 1000m);
        var discounted = p.WithDiscount(0.10);

        Assert.Equal(900m, discounted.Price);
        Assert.Equal(1000m, p.Price);  // 원본 불변
    }

    [Fact]
    public void Dog_InheritsAnimal()
    {
        var dog = new Dog("Rex", "푸들");

        Assert.Equal("Rex", dog.Name);
        Assert.Equal("Canis lupus familiaris", dog.Species);
        Assert.Contains("Rex", dog.Bark());
    }

    [Fact]
    public void Record_InheritanceEquality_DifferentTypes()
    {
        Animal animal = new Animal("Rex", "Canis lupus familiaris");
        Animal dog    = new Dog("Rex", "푸들");

        // EqualityContract가 다르므로 false
        Assert.False(animal == dog);
    }

    [Fact]
    public void Point2D_DistanceTo()
    {
        var origin = Point2D.Origin;
        var p      = new Point2D(3, 4);

        Assert.Equal(5.0, p.Distance, precision: 10);
        Assert.Equal(5.0, origin.DistanceTo(p), precision: 10);
    }

    [Fact]
    public void Velocity_RecordStruct_CopySemantics()
    {
        var v1 = new Velocity(3, 4);
        var v2 = v1;            // 값 복사
        v2 = v2.Scale(2);

        Assert.Equal(3, v1.Vx);  // 원본 불변
        Assert.Equal(6, v2.Vx);
    }

    [Fact]
    public void RGB_BlendAndInvert()
    {
        var red   = RGB.Red;
        var white = RGB.White;
        var blend = red.Blend(white);

        Assert.Equal(255, blend.R);  // (255+255)/2 = 255
        Assert.Equal(127, blend.G);  // (0+255)/2  = 127
        Assert.Equal(127, blend.B);  // (0+255)/2  = 127

        var inv = RGB.Black.Invert();
        Assert.Equal(RGB.White, inv);
    }
}
