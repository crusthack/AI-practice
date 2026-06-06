// Generator가 [AutoToString] 속성을 AutoGen 네임스페이스에 자동 생성합니다.
// Generator.csproj → OutputItemType="Analyzer" 로 참조하면 컴파일 시 주입됩니다.
using AutoGen;

namespace Consumer;

// Generator: partial class에 ToString() 자동 생성
[AutoToString]
public partial class Product
{
    public string  Name     { get; init; } = "Widget";
    public decimal Price    { get; init; } = 9.99m;
    public int     Stock    { get; init; } = 100;
}

[AutoToString]
public partial class Order
{
    public int    Id       { get; init; }
    public string Customer { get; init; } = "";
    public string Status   { get; init; } = "Pending";
}

// Analyzer 경고 트리거 예시 (빌드 시 EMPTYCAT001 발생)
public class ServiceWithIssues
{
    public void ProcessOrder(int id)
    {
        try
        {
            Console.WriteLine($"처리 중: {id}");
        }
        catch (Exception)
        {
            // 빈 catch — EMPTYCAT001 경고 발생
        }
    }
}
