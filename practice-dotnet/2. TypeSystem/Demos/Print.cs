namespace TypeSystem.Demos;

static class Print
{
    public static void Header(string title)
    {
        Console.WriteLine();
        Console.WriteLine($"{'=',1}{'=',1}{'=',1} {title} {'=',1}{'=',1}{'=',1}");
    }

    public static void Section(string title)
    {
        Console.WriteLine($"\n── {title}");
    }
}
