namespace OOP.Demos;

static class Print
{
    public static void Header(string title) =>
        Console.WriteLine($"\n=== {title} ===");

    public static void Section(string title) =>
        Console.WriteLine($"\n── {title}");

    public static void Line(string msg) =>
        Console.WriteLine($"    {msg}");
}
