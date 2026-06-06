namespace Containers.Demos;

static class Print
{
    public static void Header(string t)  => Console.WriteLine($"\n=== {t} ===");
    public static void Section(string t) => Console.WriteLine($"\n── {t}");
    public static void Line(string msg)  => Console.WriteLine($"    {msg}");
    public static void Items<T>(IEnumerable<T> items, string label = "")
        => Console.WriteLine($"    {label}{string.Join(", ", items)}");
}
