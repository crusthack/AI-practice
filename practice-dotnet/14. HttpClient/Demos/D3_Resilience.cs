using System.Net;
using Polly;
using Polly.Extensions.Http;

namespace HttpClientDemo.Demos;

static class D3_Resilience
{
    public static async Task Run()
    {
        Print.Header("3. 복원력 — 재시도 · 타임아웃 · Circuit Breaker");
        await ShowRetryPolicy();
        await ShowTimeout();
        await ShowCircuitBreaker();
        ShowPolicyWrap();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowRetryPolicy()
    {
        Print.Section("3-1. 재시도 정책 (Polly)");

        var attempts = 0;
        var handler = new FailThenSucceedHandler(failCount: 2, () =>
        {
            attempts++;
            Print.Line($"  시도 {attempts}...");
        });

        // 지수 백오프 재시도 3회
        var retryPolicy = HttpPolicyExtensions
            .HandleTransientHttpError()
            .WaitAndRetryAsync(
                retryCount: 3,
                retryAttempt => TimeSpan.FromMilliseconds(50 * Math.Pow(2, retryAttempt)),
                onRetry: (outcome, delay, attempt, _) =>
                    Print.Line($"  재시도 {attempt}: {outcome.Result?.StatusCode} → {delay.TotalMilliseconds:F0}ms 대기"));

        var client = new System.Net.Http.HttpClient(
            new PolicyHttpMessageHandler(retryPolicy) { InnerHandler = handler })
        {
            BaseAddress = new Uri("https://api.example.com")
        };

        var response = await client.GetAsync("/unstable");
        Print.Line($"  최종: {response.StatusCode}  시도 횟수: {attempts}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowTimeout()
    {
        Print.Section("3-2. 타임아웃");

        // HttpClient.Timeout — 전체 요청 타임아웃
        var slowHandler = new MockHttpHandler();
        // 응답을 의도적으로 딜레이하는 핸들러
        var delayHandler = new DelayHandler(delay: 500, innerHandler: slowHandler);
        var client = new System.Net.Http.HttpClient(delayHandler)
        {
            BaseAddress = new Uri("https://api.example.com"),
            Timeout = TimeSpan.FromMilliseconds(100) // 100ms 타임아웃
        };

        slowHandler.Enqueue(System.Net.HttpStatusCode.OK, "ok");

        try
        {
            await client.GetAsync("/slow");
            Print.Line("  타임아웃 발생 안 함 (예상치 못한 상황)");
        }
        catch (TaskCanceledException)
        {
            Print.Line($"  타임아웃 발생 (100ms 초과)");
        }

        // CancellationToken 기반 타임아웃 (더 세밀한 제어)
        using var cts = new CancellationTokenSource(50);
        var client2 = new System.Net.Http.HttpClient(new DelayHandler(200, new MockHttpHandler().Enqueue(System.Net.HttpStatusCode.OK, "ok")))
        {
            BaseAddress = new Uri("https://api.example.com")
        };

        try { await client2.GetAsync("/slow", cts.Token); }
        catch (OperationCanceledException) { Print.Line("  CancellationToken 타임아웃 동작"); }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowCircuitBreaker()
    {
        Print.Section("3-3. Circuit Breaker");

        var callCount = 0;
        var handler = new MockHttpHandler();

        // 연속 실패 시 Circuit Breaker 발동
        for (int i = 0; i < 6; i++)
            handler.Enqueue(System.Net.HttpStatusCode.InternalServerError, "error");
        handler.Enqueue(System.Net.HttpStatusCode.OK, "recovered");

        var cbPolicy = HttpPolicyExtensions
            .HandleTransientHttpError()
            .CircuitBreakerAsync(
                handledEventsAllowedBeforeBreaking: 3,
                durationOfBreak: TimeSpan.FromMilliseconds(200),
                onBreak: (_, ts) => Print.Line($"  서킷 열림: {ts.TotalMilliseconds}ms"),
                onReset: ()   => Print.Line("  서킷 닫힘 (복구)"),
                onHalfOpen:   () => Print.Line("  서킷 반열림 (테스트 중)"));

        var client = new System.Net.Http.HttpClient(
            new PolicyHttpMessageHandler(cbPolicy) { InnerHandler = handler })
        {
            BaseAddress = new Uri("https://api.example.com")
        };

        for (int i = 0; i < 5; i++)
        {
            try
            {
                callCount++;
                var r = await client.GetAsync("/service");
                Print.Line($"  요청 {i+1}: {r.StatusCode}");
            }
            catch (Polly.CircuitBreaker.BrokenCircuitException)
            {
                Print.Line($"  요청 {i+1}: CircuitBreaker 차단");
            }
            catch (Exception ex)
            {
                Print.Line($"  요청 {i+1}: {ex.GetType().Name}");
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowPolicyWrap()
    {
        Print.Section("3-4. PolicyWrap — 정책 조합 권장 패턴");

        Print.Line("  권장 정책 조합 순서 (바깥→안):");
        Print.Line("    1. Retry      (재시도)");
        Print.Line("    2. CircuitBreaker (서킷 브레이커)");
        Print.Line("    3. Timeout    (타임아웃)");
        Print.Line("    4. Fallback   (폴백 값 반환)");
        Print.Line("");
        Print.Line("  실제 적용:");
        Print.Line("    services.AddHttpClient<T>()");
        Print.Line("           .AddPolicyHandler(GetRetryPolicy())");
        Print.Line("           .AddPolicyHandler(GetCircuitBreakerPolicy())");
        Print.Line("           .AddPolicyHandler(GetTimeoutPolicy());");
    }
}

// ── 헬퍼 핸들러 ───────────────────────────────────────────────────────────────

sealed class FailThenSucceedHandler(int failCount, Action onAttempt) : HttpMessageHandler
{
    private int _attempts;
    protected override Task<HttpResponseMessage> SendAsync(HttpRequestMessage _, CancellationToken __)
    {
        onAttempt();
        _attempts++;
        var status = _attempts <= failCount
            ? System.Net.HttpStatusCode.InternalServerError
            : System.Net.HttpStatusCode.OK;
        return Task.FromResult(new HttpResponseMessage(status));
    }
}

sealed class DelayHandler(int delay, HttpMessageHandler innerHandler) : DelegatingHandler(innerHandler)
{
    protected override async Task<HttpResponseMessage> SendAsync(
        HttpRequestMessage request, CancellationToken ct)
    {
        await Task.Delay(delay, ct);
        return await base.SendAsync(request, ct);
    }
}
