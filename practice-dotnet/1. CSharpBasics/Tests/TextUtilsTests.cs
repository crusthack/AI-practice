using CSharpBasics;

namespace CSharpBasics.Tests;

public class TextUtilsTests
{
    [Theory]
    [InlineData("hello world", "Hello World")]
    [InlineData("HELLO WORLD", "Hello World")]
    [InlineData("a", "A")]
    [InlineData("", "")]
    public void TitleCase_ConvertsCorrectly(string input, string expected) =>
        Assert.Equal(expected, TextUtils.TitleCase(input));

    [Theory]
    [InlineData("racecar", true)]
    [InlineData("A man a plan a canal Panama", true)]
    [InlineData("Was it a car or a cat I saw", true)]
    [InlineData("hello", false)]
    public void IsPalindrome_DetectsCorrectly(string input, bool expected) =>
        Assert.Equal(expected, TextUtils.IsPalindrome(input));

    [Theory]
    [InlineData("Hello World", 5,  "He...")]
    [InlineData("Hello",       10, "Hello")]
    [InlineData("Hello World", 11, "Hello World")]
    public void Truncate_TruncatesCorrectly(string input, int max, string expected) =>
        Assert.Equal(expected, TextUtils.Truncate(input, max));

    [Theory]
    [InlineData("hello world", 2)]
    [InlineData("  spaces  ",  1)]
    [InlineData("",            0)]
    [InlineData("single",      1)]
    public void CountWords_CountsCorrectly(string input, int expected) =>
        Assert.Equal(expected, TextUtils.CountWords(input));
}
