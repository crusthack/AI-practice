namespace Memory.Demos;

static class Print
{
    public static void Header(string title)
    {
        Console.WriteLine();
        Console.WriteLine($"=== {title} ===");
        Console.WriteLine();
    }

    public static void Section(string title) => Console.WriteLine($"\n── {title}");

    public static void Line(string text) => Console.WriteLine($"    {text}");

    public static void Items<T>(IEnumerable<T> items, string prefix = "")
        => Console.WriteLine($"    {prefix}{string.Join(", ", items)}");
}
