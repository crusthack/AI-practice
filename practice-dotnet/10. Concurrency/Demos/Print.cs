namespace Demos;

static class Print
{
    public static void Header(string title)
    {
        Console.WriteLine();
        Console.WriteLine(new string('═', 62));
        Console.WriteLine($"  {title}");
        Console.WriteLine(new string('═', 62));
    }

    public static void Section(string title) =>
        Console.WriteLine($"\n  ─── {title}");
}
