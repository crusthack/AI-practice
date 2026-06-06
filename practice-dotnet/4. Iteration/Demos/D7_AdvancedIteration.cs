using System.Runtime.InteropServices;

namespace Iteration.Demos;

static class D7_AdvancedIteration
{
    public static void Run()
    {
        Print.Header("7. Advanced Iteration & LINQ Patterns");

        ShowTryGetNonEnumeratedCount();
        ShowCustomOperators();
        ShowLookupVsGroupBy();
        ShowSpanBackedListMutation();
    }

    static void ShowTryGetNonEnumeratedCount()
    {
        Print.Section("7-1. TryGetNonEnumeratedCount - avoid accidental enumeration");

        int[] array = [1, 2, 3, 4, 5];
        IEnumerable<int> lazy = array.Where(x => x % 2 == 1);

        Console.WriteLine($"    array count known: {array.TryGetNonEnumeratedCount(out int arrayCount)} ({arrayCount})");
        Console.WriteLine($"    lazy count known : {lazy.TryGetNonEnumeratedCount(out int lazyCount)} ({lazyCount})");
    }

    static void ShowCustomOperators()
    {
        Print.Section("7-2. Custom deferred operators - Window and Scan");

        int[] source = [1, 2, 3, 4, 5];
        var windows = AdvancedIteration.Window(source, size: 3)
            .Select(w => $"[{string.Join("+", w)}]");
        var runningSum = AdvancedIteration.Scan(source, seed: 0, (acc, x) => acc + x);

        Console.Write("    Window(3): ");
        Print.Items(windows, "");
        Console.Write("    Scan(sum): ");
        Print.Items(runningSum, "");
    }

    static void ShowLookupVsGroupBy()
    {
        Print.Section("7-3. ToLookup - materialized multi-map");

        var lookup = SampleData.Products().ToLookup(p => p.Category);

        Console.Write("    electronics: ");
        Print.Items(lookup["전자"].Select(p => p.Name), "");
        Console.WriteLine($"    missing category count: {lookup["없는분류"].Count()}");
    }

    static void ShowSpanBackedListMutation()
    {
        Print.Section("7-4. CollectionsMarshal.AsSpan - mutate List<T> storage directly");

        var values = new List<int> { 1, 2, 3, 4 };
        Span<int> span = CollectionsMarshal.AsSpan(values);

        foreach (ref int item in span)
            item *= 10;

        Console.Write("    mutated list: ");
        Print.Items(values, "");
        Console.WriteLine("    Do not Add/Remove while a span over List<T> is in use.");
    }
}

public static class AdvancedIteration
{
    public static IEnumerable<T[]> Window<T>(IEnumerable<T> source, int size)
    {
        ArgumentNullException.ThrowIfNull(source);
        ArgumentOutOfRangeException.ThrowIfLessThan(size, 1);

        var queue = new Queue<T>(size);
        foreach (T item in source)
        {
            queue.Enqueue(item);
            if (queue.Count < size)
                continue;

            yield return queue.ToArray();
            queue.Dequeue();
        }
    }

    public static IEnumerable<TAccumulate> Scan<TSource, TAccumulate>(
        IEnumerable<TSource> source,
        TAccumulate seed,
        Func<TAccumulate, TSource, TAccumulate> accumulator)
    {
        ArgumentNullException.ThrowIfNull(source);
        ArgumentNullException.ThrowIfNull(accumulator);

        TAccumulate current = seed;
        foreach (TSource item in source)
        {
            current = accumulator(current, item);
            yield return current;
        }
    }
}
