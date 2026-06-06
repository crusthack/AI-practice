using System.Text.Json;
using System.Text.Json.Serialization;

namespace StringsAndIO.Demos;

// ── 모델 ──────────────────────────────────────────────────────────────────────

public record Product(
    int Id,
    string Name,
    decimal Price,
    [property: JsonIgnore] string? InternalCode = null);

public record Order(
    int Id,
    string Customer,
    List<Product> Items,
    DateTime CreatedAt)
{
    public decimal Total => Items.Sum(p => p.Price);
}

[JsonConverter(typeof(JsonStringEnumConverter))]
public enum Status { Active, Inactive, Pending }

public class UserProfile
{
    public int Id { get; set; }
    [JsonPropertyName("full_name")] public string FullName { get; set; } = "";
    public Status Status { get; set; }
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Email { get; set; }
    public Dictionary<string, string> Metadata { get; set; } = [];
}

// ── 데모 ──────────────────────────────────────────────────────────────────────

static class D4_Json
{
    private static readonly JsonSerializerOptions PrettyOpts = new()
    {
        WriteIndented          = true,
        PropertyNamingPolicy   = JsonNamingPolicy.CamelCase,
        DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
    };

    private static readonly JsonSerializerOptions CompactOpts = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
    };

    public static void Run()
    {
        Print.Header("4. System.Text.Json — 직렬화 · 역직렬화");
        ShowBasicSerialization();
        ShowDeserializationOptions();
        ShowJsonOptions();
        ShowJsonDocument();
        ShowCustomConverter();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowBasicSerialization()
    {
        Print.Section("4-1. 기본 직렬화 / 역직렬화");

        var order = new Order(1, "홍길동",
        [
            new(101, "노트북", 1_500_000m),
            new(102, "마우스", 30_000m, InternalCode: "MC-9"),
        ],
        new DateTime(2024, 6, 15));

        // 직렬화
        string json = JsonSerializer.Serialize(order, PrettyOpts);
        Print.Line($"JSON ({json.Length}자):");
        foreach (var line in json.Split('\n').Take(8))
            Console.WriteLine("    " + line.TrimEnd());

        // 역직렬화
        var restored = JsonSerializer.Deserialize<Order>(json, PrettyOpts)!;
        Print.Line($"복원: Id={restored.Id}  Customer={restored.Customer}  Total={restored.Total:C}");
        Print.Line($"InternalCode (JsonIgnore): '{restored.Items[0].InternalCode}'");  // null
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowDeserializationOptions()
    {
        Print.Section("4-2. 역직렬화 옵션");

        // snake_case → C# PascalCase 매핑 (PropertyNameCaseInsensitive)
        string json = """
            {
              "full_name": "Kim Chul-su",
              "status": "Active",
              "id": 42,
              "metadata": { "dept": "R&D", "level": "senior" }
            }
            """;

        var opts = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };
        var user = JsonSerializer.Deserialize<UserProfile>(json, opts)!;
        Print.Line($"  Id={user.Id}  FullName={user.FullName}  Status={user.Status}");
        Print.Line($"  Metadata: {string.Join(", ", user.Metadata.Select(kv => $"{kv.Key}={kv.Value}"))}");

        // nullable 처리
        string withNull = """{"id": 1, "full_name": "test", "email": null}""";
        var userWithNull = JsonSerializer.Deserialize<UserProfile>(withNull, opts)!;
        Print.Line($"  Email (null 허용): {userWithNull.Email ?? "(null)"}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowJsonOptions()
    {
        Print.Section("4-3. 직렬화 옵션 — 숫자 · 날짜 · 순환 참조");

        // 숫자를 문자열로 → JSON 정밀도 보존
        var opts = new JsonSerializerOptions
        {
            NumberHandling = JsonNumberHandling.AllowReadingFromString
                           | JsonNumberHandling.WriteAsString,
        };
        string numStr = JsonSerializer.Serialize(new { amount = 9_999_999.99m }, opts);
        Print.Line($"  숫자→문자열: {numStr}");

        // 날짜 형식
        var dtOpts = new JsonSerializerOptions
        {
            WriteIndented = false,
        };
        string dtJson = JsonSerializer.Serialize(
            new { createdAt = new DateTime(2024, 6, 15, 12, 0, 0, DateTimeKind.Utc) },
            dtOpts);
        Print.Line($"  날짜 기본:  {dtJson}");

        // Utf8JsonWriter — 저수준 직렬화 (할당 최소화)
        using var ms = new System.IO.MemoryStream();
        using (var writer = new Utf8JsonWriter(ms, new JsonWriterOptions { Indented = false }))
        {
            writer.WriteStartObject();
            writer.WriteString("name", "홍길동");
            writer.WriteNumber("age", 30);
            writer.WriteEndObject();
        }
        Print.Line($"  Utf8JsonWriter: {System.Text.Encoding.UTF8.GetString(ms.ToArray())}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowJsonDocument()
    {
        Print.Section("4-4. JsonDocument · JsonElement — 동적 파싱");

        string json = """
            {
              "users": [
                {"id": 1, "name": "Alice", "score": 95.5},
                {"id": 2, "name": "Bob",   "score": 87.0}
              ],
              "total": 2
            }
            """;

        using var doc = JsonDocument.Parse(json);
        var root = doc.RootElement;

        int total = root.GetProperty("total").GetInt32();
        Print.Line($"  total: {total}");

        foreach (var user in root.GetProperty("users").EnumerateArray())
        {
            int id     = user.GetProperty("id").GetInt32();
            string name = user.GetProperty("name").GetString()!;
            double score = user.GetProperty("score").GetDouble();
            Print.Line($"  user[{id}]: {name}  score={score}");
        }

        // JsonElement.TryGetProperty
        if (root.TryGetProperty("missing", out var missing))
            Print.Line($"  missing: {missing}");
        else
            Print.Line($"  TryGetProperty('missing'): 없음");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowCustomConverter()
    {
        Print.Section("4-5. 커스텀 JsonConverter");

        var opts = new JsonSerializerOptions();
        opts.Converters.Add(new MoneyConverter());

        var invoice = new { amount = new Money(15000, "KRW"), tax = new Money(1500, "KRW") };
        string json = JsonSerializer.Serialize(invoice, opts);
        Print.Line($"  직렬화: {json}");

        var deserialized = JsonSerializer.Deserialize<Dictionary<string, Money>>(json, opts)!;
        Print.Line($"  역직렬화: amount={deserialized["amount"]}  tax={deserialized["tax"]}");
    }
}

// ── Money 커스텀 타입 + Converter ─────────────────────────────────────────────

public readonly record struct Money(decimal Amount, string Currency)
{
    public override string ToString() => $"{Amount:N0} {Currency}";
}

public class MoneyConverter : JsonConverter<Money>
{
    public override Money Read(ref Utf8JsonReader reader, Type _, JsonSerializerOptions __)
    {
        string str = reader.GetString() ?? "0 KRW";
        var parts  = str.Split(' ', 2);
        return new Money(decimal.Parse(parts[0]), parts.Length > 1 ? parts[1] : "KRW");
    }

    public override void Write(Utf8JsonWriter writer, Money value, JsonSerializerOptions _) =>
        writer.WriteStringValue($"{value.Amount} {value.Currency}");
}
