namespace CSharpBasics;

public static class TextUtils
{
    public static string TitleCase(string text)
    {
        if (string.IsNullOrWhiteSpace(text)) return text;
        return string.Join(' ', text.Split(' ')
            .Select(w => w.Length == 0 ? w : char.ToUpper(w[0]) + w[1..].ToLower()));
    }

    public static bool IsPalindrome(string text)
    {
        var chars = text.Where(char.IsLetterOrDigit)
                        .Select(char.ToLower)
                        .ToArray();
        return chars.SequenceEqual(chars.Reverse());
    }

    public static string Truncate(string text, int maxLength, string ellipsis = "...")
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(ellipsis);
        ArgumentOutOfRangeException.ThrowIfNegative(maxLength);

        if (ellipsis.Length > maxLength)
            throw new ArgumentException("Ellipsis length cannot exceed maxLength.", nameof(ellipsis));

        if (text.Length <= maxLength) return text;
        return text[..(maxLength - ellipsis.Length)] + ellipsis;
    }

    public static int CountWords(string text) =>
        string.IsNullOrWhiteSpace(text)
            ? 0
            : text.Split(' ', StringSplitOptions.RemoveEmptyEntries).Length;

    public static string Repeat(string text, int times) =>
        string.Concat(Enumerable.Repeat(text, times));
}
