using _3._Logger.Services;
using Microsoft.Extensions.Logging;

Console.OutputEncoding = System.Text.Encoding.UTF8;

// ── LoggerFactory로 직접 구성 (Host 없이 간결하게)
using var loggerFactory = LoggerFactory.Create(builder =>
    builder
        .SetMinimumLevel(LogLevel.Trace)
        .AddSimpleConsole(opts =>
        {
            opts.SingleLine      = true;
            opts.IncludeScopes   = true;
            opts.TimestampFormat = "HH:mm:ss ";
        }));

var logger = loggerFactory.CreateLogger<Program>();

// ── 1. 로그 레벨
Console.WriteLine("\n=== 1. 로그 레벨 (Trace → Critical) ===");
logger.LogTrace("Trace: 가장 상세한 진단 (프로덕션에서는 비활성)");
logger.LogDebug("Debug: 개발 중 디버깅 정보");
logger.LogInformation("Information: 일반 흐름 ({Environment})",
    Environment.GetEnvironmentVariable("DOTNET_ENVIRONMENT") ?? "Production");
logger.LogWarning("Warning: 비정상이지만 치명적이지 않음");
logger.LogError("Error: 현재 작업 실패");
logger.LogCritical("Critical: 전체 시스템에 영향");

// ── 2. 구조적 로깅 (Structured Logging)
Console.WriteLine("\n=== 2. 구조적 로깅 — 속성(Property)으로 저장 ===");
logger.LogInformation("주문 처리: OrderId={OrderId}, Amount={Amount:C}", 42, 1500.0m);
// JSON 로그 수집기(ELK, Seq)에서 OrderId=42 로 필터링 가능

// ── 3. EventId로 이벤트 분류
Console.WriteLine("\n=== 3. EventId — 로그 이벤트 분류 ===");
var StartEvent = new EventId(1001, "OrderStarted");
var EndEvent   = new EventId(1002, "OrderCompleted");
logger.LogInformation(StartEvent,  "주문 시작 이벤트 (ID={EventId})", StartEvent.Id);
logger.LogInformation(EndEvent,    "주문 완료 이벤트");

// ── 4. BeginScope — 컨텍스트 전파
Console.WriteLine("\n=== 4. BeginScope — 스코프 내 모든 로그에 컨텍스트 포함 ===");
using (logger.BeginScope(
    "OrderId={OrderId} CustomerId={CustomerId} RequestId={RequestId}",
    99,
    "C-007",
    Guid.NewGuid().ToString("N")[..8]))
{
    logger.LogInformation("스코프 내 로그 — OrderId/CustomerId 자동 포함");
    logger.LogDebug("디버그도 같은 스코프에 속함");
}
logger.LogInformation("스코프 밖 — 추가 컨텍스트 없음");

// ── 5. 서비스별 카테고리 로거
Console.WriteLine("\n=== 5. 서비스별 로거 (카테고리 = 클래스 이름) ===");
var orderService = new OrderService(loggerFactory.CreateLogger<OrderService>());
orderService.ProcessOrder(100, "C-001",   250m);
orderService.ProcessOrder(101, "C-002", 1_500_000m); // Warning 발생
try
{
    orderService.ProcessOrder(102, "C-003", -50m);   // Error 발생
}
catch (ArgumentException ex)
{
    logger.LogDebug("예외 캐치됨: {Message}", ex.Message);
}

// ── 6. IsEnabled 가드 — 불필요한 포매팅 방지
Console.WriteLine("\n=== 6. IsEnabled 가드 — 비용 큰 표현식 보호 ===");
if (logger.IsEnabled(LogLevel.Debug))
{
    var expensiveData = string.Join(", ", Enumerable.Range(0, 50));
    logger.LogDebug("비용 큰 데이터: {Data}", expensiveData);
}
else
{
    Console.WriteLine("  Debug 레벨 비활성 → 포매팅 생략됨");
}

Console.WriteLine("\n로거 데모 완료.");
