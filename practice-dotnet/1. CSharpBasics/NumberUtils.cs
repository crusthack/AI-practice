namespace CSharpBasics;

public static class NumberUtils
{
    public static bool IsPrime(int n)
    {
        if (n < 2) return false;
        if (n == 2) return true;
        if (n % 2 == 0) return false;
        for (int i = 3; i * i <= n; i += 2)
            if (n % i == 0) return false;
        return true;
    }

    // 에라토스테네스의 체 (Sieve of Eratosthenes)
    public static IEnumerable<int> PrimesUpTo(int max)
    {
        if (max < 2) return [];
        var sieve = new bool[max + 1];
        Array.Fill(sieve, true);
        sieve[0] = sieve[1] = false;
        for (int i = 2; i * i <= max; i++)
            if (sieve[i])
                for (int j = i * i; j <= max; j += i)
                    sieve[j] = false;
        return Enumerable.Range(2, max - 1).Where(i => sieve[i]);
    }

    public static long Fibonacci(int n)
    {
        if (n < 0) throw new ArgumentOutOfRangeException(nameof(n));
        if (n <= 1) return n;
        long a = 0, b = 1;
        for (int i = 2; i <= n; i++)
            (a, b) = (b, a + b);
        return b;
    }

    public static IEnumerable<long> FibonacciSequence(int count)
    {
        if (count <= 0) yield break;
        long a = 0, b = 1;
        yield return a;
        if (count == 1) yield break;
        yield return b;
        for (int i = 2; i < count; i++)
        {
            (a, b) = (b, a + b);
            yield return b;
        }
    }
}
