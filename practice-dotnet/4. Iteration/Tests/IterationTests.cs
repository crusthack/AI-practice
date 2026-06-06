using System.Collections.Immutable;
using Iteration;
using Iteration.Demos;

// ────────────────────────────────────────────────────────────────────────────
// Module 6 — Iteration 테스트
// ────────────────────────────────────────────────────────────────────────────

public class ModelTests
{
    [Fact]
    public void Student_CompareTo_ByGpa()
    {
        var low  = new Student("A", 1, 2.5, "CS");
        var high = new Student("B", 1, 3.9, "CS");

        Assert.True(low.CompareTo(high) < 0);
        Assert.True(high.CompareTo(low) > 0);
        Assert.Equal(0, low.CompareTo(low));
    }

    [Fact]
    public void Student_CompareTo_NullIsLess()
    {
        var s = new Student("A", 1, 3.0, "CS");
        Assert.True(s.CompareTo(null) > 0);
    }

    [Fact]
    public void Product_IsAvailable_True()
    {
        var p = new Product("노트북", "전자", 1_200_000m, 15);
        Assert.True(p.IsAvailable);
    }

    [Fact]
    public void Product_IsAvailable_False_WhenNoStock()
    {
        var p = new Product("모니터", "전자", 450_000m, 0);
        Assert.False(p.IsAvailable);
    }

    [Fact]
    public void OrderItem_Subtotal_Computed()
    {
        var item = new OrderItem("노트북", 1_200_000m, 3);
        Assert.Equal(3_600_000m, item.Subtotal);
    }

    [Fact]
    public void Student_Record_Equality()
    {
        var a = new Student("Alice", 4, 3.9, "CS");
        var b = new Student("Alice", 4, 3.9, "CS");
        Assert.Equal(a, b);
        Assert.True(a == b);
    }
}

public class NumberRangeTests
{
    [Fact]
    public void NumberRange_EnumeratesCorrectly()
    {
        var range = new NumberRange(1, 5);
        Assert.Equal(new[] { 1, 2, 3, 4, 5 }, range.ToArray());
    }

    [Fact]
    public void NumberRange_EmptyWhenStartEqualsEnd()
    {
        var range = new NumberRange(3, 3);
        Assert.Single(range);
        Assert.Equal(3, range.First());
    }

    [Fact]
    public void NumberRange_IndependentEnumerators()
    {
        var range = new NumberRange(1, 5);
        var e1 = range.GetEnumerator();
        var e2 = range.GetEnumerator();

        e1.MoveNext(); e1.MoveNext();  // e1 → 2
        e2.MoveNext();                  // e2 → 1

        Assert.Equal(2, e1.Current);
        Assert.Equal(1, e2.Current);
        e1.Dispose(); e2.Dispose();
    }

    [Fact]
    public void NumberRange_MultipleEnumeration()
    {
        var range = new NumberRange(1, 3);
        Assert.Equal(6, range.Sum());
        Assert.Equal(6, range.Sum());  // 두 번째 열거도 같은 결과
    }

    [Fact]
    public void NumberRange_LinqCompatible()
    {
        var range = new NumberRange(1, 10);
        Assert.Equal(new[] { 2, 4, 6, 8, 10 }, range.Where(x => x % 2 == 0).ToArray());
        Assert.Equal(10, range.Count());
        Assert.Equal(55, range.Sum());
    }

    [Fact]
    public void NumberRange_Reset_RestartsFromBeginning()
    {
        var e = new NumberRangeEnumerator(1, 3);
        e.MoveNext(); e.MoveNext();  // current = 2
        e.Reset();
        e.MoveNext();
        Assert.Equal(1, e.Current);
        e.Dispose();
    }
}

public class InfiniteCounterTests
{
    [Fact]
    public void InfiniteCounter_ProducesCorrectSequence()
    {
        var counter = new InfiniteCounter(start: 0, step: 10);
        Assert.Equal(new[] { 0, 10, 20, 30, 40 }, counter.Take(5).ToArray());
    }

    [Fact]
    public void InfiniteCounter_DifferentStartAndStep()
    {
        var counter = new InfiniteCounter(start: 5, step: 3);
        Assert.Equal(new[] { 5, 8, 11, 14, 17 }, counter.Take(5).ToArray());
    }

    [Fact]
    public void InfiniteCounter_SupportsLinq()
    {
        var counter = new InfiniteCounter(start: 1, step: 1);
        int sum = counter.Take(100).Sum();
        Assert.Equal(5050, sum);  // 1+2+...+100
    }

    [Fact]
    public void InfiniteCounter_FilterAndTake()
    {
        var counter = new InfiniteCounter(start: 0, step: 1);
        var divisibleBy3 = counter.Where(n => n % 3 == 0).Take(5).ToArray();
        Assert.Equal(new[] { 0, 3, 6, 9, 12 }, divisibleBy3);
    }
}

public class TemperatureTests
{
    [Fact]
    public void Temperature_CompareTo_Ordering()
    {
        var cold = new Temperature(0);
        var warm = new Temperature(25);
        var hot  = new Temperature(37.5);

        Assert.True(cold.CompareTo(warm) < 0);
        Assert.True(hot.CompareTo(warm) > 0);
        Assert.Equal(0, warm.CompareTo(new Temperature(25)));
    }

    [Fact]
    public void Temperature_ArraySort()
    {
        var temps = new[] { new Temperature(37.5), new Temperature(36.5), new Temperature(38.1) };
        Array.Sort(temps);
        Assert.Equal(36.5, temps[0].Celsius);
        Assert.Equal(37.5, temps[1].Celsius);
        Assert.Equal(38.1, temps[2].Celsius);
    }

    [Fact]
    public void Temperature_CompareTo_Null()
    {
        var t = new Temperature(36.5);
        Assert.True(t.CompareTo(null) > 0);
    }
}

public class SampleDataTests
{
    [Fact]
    public void SampleData_Students_HasEight()
    {
        Assert.Equal(8, SampleData.Students().Count);
    }

    [Fact]
    public void SampleData_Products_HasTen()
    {
        Assert.Equal(10, SampleData.Products().Count);
    }

    [Fact]
    public void SampleData_Students_AllDepartmentsPresent()
    {
        var depts = SampleData.Students().Select(s => s.Department).Distinct().OrderBy(d => d).ToList();
        Assert.Contains("CS", depts);
        Assert.Contains("Math", depts);
    }

    [Fact]
    public void SampleData_Products_SomeUnavailable()
    {
        Assert.Contains(SampleData.Products(), p => !p.IsAvailable);
        Assert.Contains(SampleData.Products(), p => p.IsAvailable);
    }
}

public class ImmutableCollectionTests
{
    [Fact]
    public void ImmutableArray_DoesNotMutateOriginal()
    {
        var ia  = ImmutableArray.Create(1, 2, 3);
        var ia2 = ia.Add(4);

        Assert.Equal(3, ia.Length);   // 원본 불변
        Assert.Equal(4, ia2.Length);
    }

    [Fact]
    public void ImmutableList_Insert_CreatesNewList()
    {
        var il  = ImmutableList.Create("A", "B", "C");
        var il2 = il.Insert(1, "X");

        Assert.Equal(3, il.Count);
        Assert.Equal(4, il2.Count);
        Assert.Equal("X", il2[1]);
    }

    [Fact]
    public void ImmutableDictionary_SetItem_Overwrites()
    {
        var id = ImmutableDictionary<string, int>.Empty
            .Add("x", 1)
            .SetItem("x", 99);

        Assert.Equal(99, id["x"]);
    }
}

public class AdvancedIterationTests
{
    [Fact]
    public void Window_ReturnsSlidingWindows()
    {
        var windows = AdvancedIteration.Window([1, 2, 3, 4], 3).ToArray();

        Assert.Equal(2, windows.Length);
        Assert.Equal(new[] { 1, 2, 3 }, windows[0]);
        Assert.Equal(new[] { 2, 3, 4 }, windows[1]);
    }

    [Fact]
    public void Window_ThrowsForInvalidSize()
    {
        Assert.Throws<ArgumentOutOfRangeException>(() =>
            AdvancedIteration.Window([1, 2, 3], 0).ToList());
    }

    [Fact]
    public void Scan_ReturnsRunningAccumulation()
    {
        var result = AdvancedIteration.Scan([1, 2, 3, 4], 0, (sum, x) => sum + x);

        Assert.Equal(new[] { 1, 3, 6, 10 }, result);
    }
}
