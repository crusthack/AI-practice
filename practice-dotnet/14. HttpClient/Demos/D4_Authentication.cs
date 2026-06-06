using System.Net.Http.Headers;
using System.Text;
using System.Text.Json;

namespace HttpClientDemo.Demos;

static class D4_Authentication
{
    public static async Task Run()
    {
        Print.Header("4. 인증 패턴 — Bearer · API Key · Basic · OAuth2 토큰 갱신");
        await ShowBearer();
        await ShowApiKey();
        await ShowBasicAuth();
        await ShowTokenRefresh();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowBearer()
    {
        Print.Section("4-1. Bearer 토큰");

        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, """{"user":"admin","role":"superuser"}""");

        using var client = new System.Net.Http.HttpClient(handler)
        {
            BaseAddress = new Uri("https://api.example.com")
        };
        client.DefaultRequestHeaders.Authorization =
            new AuthenticationHeaderValue("Bearer", "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...");

        var response = await client.GetAsync("/me");
        var authHeader = handler.SentRequests[0].Headers.Authorization;
        Print.Line($"  Bearer 전송: {authHeader?.Scheme} {authHeader?.Parameter?[..Math.Min(20, authHeader.Parameter?.Length ?? 0)]}...");
        Print.Line($"  응답: {response.StatusCode}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowApiKey()
    {
        Print.Section("4-2. API Key — 헤더 / 쿼리스트링");

        // 헤더 방식
        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, """{"result":"ok"}""")
            .Enqueue(System.Net.HttpStatusCode.OK, """{"result":"ok"}""");

        using var client = new System.Net.Http.HttpClient(handler)
        {
            BaseAddress = new Uri("https://api.example.com")
        };

        // 헤더에 API Key 추가
        client.DefaultRequestHeaders.Add("X-API-Key", "sk-1234567890abcdef");
        var r1 = await client.GetAsync("/data");
        Print.Line($"  헤더 방식: {r1.StatusCode}  key={handler.SentRequests[0].Headers.GetValues("X-API-Key").First()}");

        // 쿼리스트링 방식 (URL에 포함)
        var r2 = await client.GetAsync("/data?api_key=sk-1234567890abcdef");
        Print.Line($"  쿼리스트링 방식: {r2.StatusCode}  uri={handler.SentRequests[1].RequestUri}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowBasicAuth()
    {
        Print.Section("4-3. Basic Authentication");

        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, """{"authenticated":true}""");

        using var client = new System.Net.Http.HttpClient(handler)
        {
            BaseAddress = new Uri("https://api.example.com")
        };

        var credentials = Convert.ToBase64String(Encoding.UTF8.GetBytes("username:password"));
        client.DefaultRequestHeaders.Authorization =
            new AuthenticationHeaderValue("Basic", credentials);

        var response = await client.GetAsync("/protected");
        var sentAuth  = handler.SentRequests[0].Headers.Authorization;
        var decoded   = Encoding.UTF8.GetString(Convert.FromBase64String(sentAuth?.Parameter ?? ""));
        Print.Line($"  Basic 전송: {sentAuth?.Scheme} (decoded={decoded})");
        Print.Line($"  응답: {response.StatusCode}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowTokenRefresh()
    {
        Print.Section("4-4. OAuth2 토큰 자동 갱신 (DelegatingHandler 패턴)");

        // 401 → 토큰 갱신 → 재시도
        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.Unauthorized, """{"error":"token_expired"}""")      // 최초 요청 실패
            .Enqueue(System.Net.HttpStatusCode.OK, """{"access_token":"new_token_xyz","expires_in":3600}""") // 토큰 갱신
            .Enqueue(System.Net.HttpStatusCode.OK, """{"data":"secret"}""");                       // 재시도 성공

        var tokenStore = new TokenStore { AccessToken = "old_expired_token" };
        var refreshHandler = new TokenRefreshHandler(tokenStore, handler);

        using var client = new System.Net.Http.HttpClient(refreshHandler)
        {
            BaseAddress = new Uri("https://api.example.com")
        };

        var response = await client.GetAsync("/protected-resource");
        Print.Line($"  최종 응답: {response.StatusCode}");
        Print.Line($"  갱신된 토큰: {tokenStore.AccessToken}");
        Print.Line($"  총 요청 수: {handler.SentRequests.Count} (원본1 + 갱신1 + 재시도1)");
    }
}

// ── 토큰 갱신 구현 ─────────────────────────────────────────────────────────────

public class TokenStore
{
    public string AccessToken { get; set; } = "";
}

public class TokenRefreshHandler(TokenStore tokenStore, HttpMessageHandler innerHandler)
    : DelegatingHandler(innerHandler)
{
    protected override async Task<HttpResponseMessage> SendAsync(
        HttpRequestMessage request, CancellationToken ct)
    {
        request.Headers.Authorization =
            new AuthenticationHeaderValue("Bearer", tokenStore.AccessToken);

        var response = await base.SendAsync(request, ct);

        if (response.StatusCode == System.Net.HttpStatusCode.Unauthorized)
        {
            // 토큰 갱신 요청
            var refreshRequest = new HttpRequestMessage(HttpMethod.Post, "/oauth/token");
            refreshRequest.Content = new FormUrlEncodedContent([
                new("grant_type", "refresh_token"),
                new("refresh_token", "refresh_token_value"),
            ]);
            var refreshResponse = await base.SendAsync(refreshRequest, ct);

            if (refreshResponse.IsSuccessStatusCode)
            {
                var tokenJson = await refreshResponse.Content.ReadAsStringAsync(ct);
                var token = JsonSerializer.Deserialize<JsonElement>(tokenJson);
                tokenStore.AccessToken = token.GetProperty("access_token").GetString() ?? "";

                // 원본 요청 재시도
                var retryRequest = new HttpRequestMessage(request.Method, request.RequestUri);
                retryRequest.Headers.Authorization =
                    new AuthenticationHeaderValue("Bearer", tokenStore.AccessToken);
                return await base.SendAsync(retryRequest, ct);
            }
        }

        return response;
    }
}
