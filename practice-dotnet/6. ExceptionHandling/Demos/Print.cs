namespace ExceptionHandling.Demos;

static class Print
{
    public static void Header(string title) =>
        Console.WriteLine($"\n=== {title} ===");

    public static void Section(string title) =>
        Console.WriteLine($"\n── {title}");

    public static void Line(string msg) =>
        Console.WriteLine($"  {msg}");

    public static void Items<T>(IEnumerable<T> items, string prefix = "    ") =>
        Console.WriteLine(prefix + string.Join(", ", items));
}
