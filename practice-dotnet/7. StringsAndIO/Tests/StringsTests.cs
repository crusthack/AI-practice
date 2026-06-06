using System.Text;
using System.Text.Json;
using StringsAndIO.Demos;
using Xunit;

public class StringTests
{
    [Fact] public void StringBuilder_Append_Works()
    {
        var sb = new StringBuilder();
        sb.Append("A").Append("B").Append("C");
        Assert.Equal("ABC", sb.ToString());
    }

    [Fact] public void StringBuilder_Insert_Works()
    {
        var sb = new StringBuilder("Hello World");
        sb.Insert(5, ",");
        Assert.Equal("Hello, World", sb.ToString());
    }

    [Fact] public void String_Format_Alignment()
    {
        string result = $"{"A",5}";
        Assert.Equal("    A", result);
    }

    [Fact] public void SpanChar_Trim_NoCopy()
    {
        ReadOnlySpan<char> span = "  hello  ".AsSpan();
        Assert.Equal("hello", span.Trim().ToString());
    }

    [Fact] public void String_Split_RemoveEmpty()
    {
        var parts = "a,,b,,c".Split(',', StringSplitOptions.RemoveEmptyEntries);
        Assert.Equal(3, parts.Length);
    }
}

public class DateTimeTests
{
    [Fact] public void DateTimeOffset_SameInstant_Equal()
    {
        var kst = new DateTimeOffset(2024, 1, 1, 12, 0, 0, TimeSpan.FromHours(9));
        var utc = new DateTimeOffset(2024, 1, 1, 3, 0, 0, TimeSpan.Zero);
        Assert.Equal(kst, utc);
    }

    [Fact] public void DateOnly_AddDays_Works()
    {
        var d = new DateOnly(2024, 1, 30);
        Assert.Equal(new DateOnly(2024, 2, 1), d.AddDays(2));
    }

    [Fact] public void TimeSpan_FromHours_Total()
    {
        var ts = TimeSpan.FromHours(1.5);
        Assert.Equal(90, ts.TotalMinutes);
    }

    [Fact] public void DateTime_TryParseExact_ISO()
    {
        bool ok = DateTime.TryParseExact("2024-06-15",
            "yyyy-MM-dd",
            System.Globalization.CultureInfo.InvariantCulture,
            System.Globalization.DateTimeStyles.None,
            out var dt);
        Assert.True(ok);
        Assert.Equal(2024, dt.Year);
        Assert.Equal(6, dt.Month);
    }
}

public class FileIOTests
{
    private string TempDir => Path.Combine(Path.GetTempPath(), "DotNetPractice_Test_" + Guid.NewGuid().ToString("N")[..8]);

    [Fact] public void File_WriteRead_RoundTrip()
    {
        var dir  = TempDir;
        Directory.CreateDirectory(dir);
        try
        {
            var path = Path.Combine(dir, "test.txt");
            File.WriteAllText(path, "hello");
            Assert.Equal("hello", File.ReadAllText(path));
        }
        finally { Directory.Delete(dir, true); }
    }

    [Fact] public void Path_Combine_IsOsIndependent()
    {
        string result = Path.Combine("base", "sub", "file.txt");
        Assert.Contains("file.txt", result);
        Assert.Contains("sub", result);
    }

    [Fact] public void Directory_Enumerate_Finds_Files()
    {
        var dir = TempDir;
        Directory.CreateDirectory(dir);
        try
        {
            File.WriteAllText(Path.Combine(dir, "a.txt"), "A");
            File.WriteAllText(Path.Combine(dir, "b.txt"), "B");
            var files = Directory.GetFiles(dir, "*.txt");
            Assert.Equal(2, files.Length);
        }
        finally { Directory.Delete(dir, true); }
    }
}

public class JsonTests
{
    [Fact] public void Serialize_Deserialize_RoundTrip()
    {
        var p = new Product(1, "Test", 9.99m);
        var opts = new JsonSerializerOptions { PropertyNamingPolicy = JsonNamingPolicy.CamelCase };
        string json = JsonSerializer.Serialize(p, opts);
        var restored = JsonSerializer.Deserialize<Product>(json, opts)!;
        Assert.Equal(p.Id, restored.Id);
        Assert.Equal(p.Price, restored.Price);
    }

    [Fact] public void JsonIgnore_ExcludesProperty()
    {
        var p = new Product(1, "Test", 9.99m, InternalCode: "SEC");
        string json = JsonSerializer.Serialize(p);
        Assert.DoesNotContain("internalCode", json, StringComparison.OrdinalIgnoreCase);
        Assert.DoesNotContain("SEC", json);
    }

    [Fact] public void JsonDocument_ParseAndNavigate()
    {
        string json = """{"name":"test","count":42}""";
        using var doc = JsonDocument.Parse(json);
        Assert.Equal("test", doc.RootElement.GetProperty("name").GetString());
        Assert.Equal(42, doc.RootElement.GetProperty("count").GetInt32());
    }

    [Fact] public void Money_CustomConverter_RoundTrip()
    {
        var opts = new JsonSerializerOptions();
        opts.Converters.Add(new MoneyConverter());
        var m = new Money(1000m, "KRW");
        string json = JsonSerializer.Serialize(m, opts);
        var back = JsonSerializer.Deserialize<Money>(json, opts);
        Assert.Equal(m.Amount, back.Amount);
        Assert.Equal(m.Currency, back.Currency);
    }
}

public class EncodingTests
{
    [Fact] public void UTF8_GetBytes_GetString_RoundTrip()
    {
        string text = "안녕하세요 Hello";
        byte[] bytes = Encoding.UTF8.GetBytes(text);
        Assert.Equal(text, Encoding.UTF8.GetString(bytes));
    }

    [Fact] public void Base64_RoundTrip()
    {
        byte[] data = [1, 2, 3, 255, 0];
        string b64 = Convert.ToBase64String(data);
        byte[] back = Convert.FromBase64String(b64);
        Assert.Equal(data, back);
    }

    [Fact] public void SHA256_Deterministic()
    {
        byte[] data = Encoding.UTF8.GetBytes("hello");
        byte[] h1   = System.Security.Cryptography.SHA256.HashData(data);
        byte[] h2   = System.Security.Cryptography.SHA256.HashData(data);
        Assert.Equal(h1, h2);
    }
}
