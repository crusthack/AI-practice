using CSharpBasics;

namespace CSharpBasics.Tests;

public class NumberUtilsTests
{
    [Theory]
    [InlineData(2,  true)]
    [InlineData(3,  true)]
    [InlineData(4,  false)]
    [InlineData(17, true)]
    [InlineData(1,  false)]
    [InlineData(0,  false)]
    [InlineData(-1, false)]
    public void IsPrime_DetectsCorrectly(int n, bool expected) =>
        Assert.Equal(expected, NumberUtils.IsPrime(n));

    [Fact]
    public void PrimesUpTo_ReturnsCorrectPrimes()
    {
        var primes = NumberUtils.PrimesUpTo(20).ToList();
        Assert.Equal([2, 3, 5, 7, 11, 13, 17, 19], primes);
    }

    [Fact]
    public void PrimesUpTo_LessThanTwo_ReturnsEmpty() =>
        Assert.Empty(NumberUtils.PrimesUpTo(1));

    [Theory]
    [InlineData(0,  0L)]
    [InlineData(1,  1L)]
    [InlineData(6,  8L)]
    [InlineData(10, 55L)]
    public void Fibonacci_ReturnsCorrectValue(int n, long expected) =>
        Assert.Equal(expected, NumberUtils.Fibonacci(n));

    [Fact]
    public void FibonacciSequence_ReturnsCorrectSequence()
    {
        var seq = NumberUtils.FibonacciSequence(8).ToList();
        Assert.Equal([0L, 1L, 1L, 2L, 3L, 5L, 8L, 13L], seq);
    }

    [Fact]
    public void Fibonacci_NegativeInput_Throws() =>
        Assert.Throws<ArgumentOutOfRangeException>(() => NumberUtils.Fibonacci(-1));
}
