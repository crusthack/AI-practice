using System.Net;
using System.Text;
using System.Text.Json;

namespace HttpClientDemo.Demos;

public record GitHubUser(string Login, string Name, int PublicRepos);
public record JsonPlaceholderPost(int UserId, int Id, string Title, string Body);
public record CreatePostDto(string Title, string Body, int UserId);

static class D1_HttpClientBasics
{
    private static readonly JsonSerializerOptions JsonOpts = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.SnakeCaseLower
    };

    public static async Task Run()
    {
        Print.Header("1. HttpClient 기본 — GET · POST · 응답 처리");
        await ShowGetRequest();
        await ShowPostRequest();
        await ShowResponseHandling();
        await ShowHeaders();
        ShowDisposalPattern();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowGetRequest()
    {
        Print.Section("1-1. GET 요청 — ReadAsStringAsync · ReadFromJsonAsync");

        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, """
                {"login":"octocat","name":"The Octocat","public_repos":8}
                """);

        using var client = new System.Net.Http.HttpClient(handler)
        {
            BaseAddress = new Uri("https://api.github.com")
        };
        client.DefaultRequestHeaders.Add("User-Agent", "DotNetPractice/1.0");

        // 문자열로 읽기
        var rawJson = await client.GetStringAsync("/users/octocat");
        Print.Line($"raw JSON: {rawJson[..Math.Min(60, rawJson.Length)]}...");

        // 타입으로 역직렬화
        handler.Enqueue(System.Net.HttpStatusCode.OK, """
            {"login":"octocat","name":"The Octocat","public_repos":8}
            """);
        var user = await client.GetFromJsonAsync<GitHubUser>("/users/octocat", JsonOpts);
        Print.Line($"역직렬화: Login={user?.Login}  Repos={user?.PublicRepos}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowPostRequest()
    {
        Print.Section("1-2. POST 요청 — JSON 직렬화 · FormData");

        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.Created, """
                {"userId":1,"id":101,"title":"Test Post","body":"Test body"}
                """);

        using var client = new System.Net.Http.HttpClient(handler)
        {
            BaseAddress = new Uri("https://jsonplaceholder.typicode.com")
        };

        // PostAsJsonAsync — 직렬화 + 전송
        var newPost = new CreatePostDto("Test Post", "Test body", 1);
        var response = await client.PostAsJsonAsync("/posts", newPost);
        response.EnsureSuccessStatusCode();
        var created = await response.Content.ReadFromJsonAsync<JsonPlaceholderPost>(JsonOpts);
        Print.Line($"생성: Id={created?.Id}  Title={created?.Title}");

        // 수동 JSON 직렬화
        handler.Enqueue(System.Net.HttpStatusCode.OK, """{"result":"ok"}""");
        var json    = JsonSerializer.Serialize(new { key = "value" });
        var content = new StringContent(json, Encoding.UTF8, "application/json");
        var r2 = await client.PostAsync("/endpoint", content);
        Print.Line($"수동 POST: {r2.StatusCode}");

        // Form data
        handler.Enqueue(System.Net.HttpStatusCode.OK, """{"token":"abc123"}""");
        var form = new FormUrlEncodedContent([
            new("grant_type", "password"),
            new("username", "user"),
            new("password", "pass"),
        ]);
        var r3 = await client.PostAsync("/token", form);
        Print.Line($"Form POST: {r3.StatusCode}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowResponseHandling()
    {
        Print.Section("1-3. 응답 처리 — StatusCode · EnsureSuccessStatusCode");

        var cases = new[]
        {
            (System.Net.HttpStatusCode.OK,                  """{"data":"success"}"""),
            (System.Net.HttpStatusCode.NotFound,            """{"error":"not found"}"""),
            (System.Net.HttpStatusCode.InternalServerError, """{"error":"server error"}"""),
        };

        foreach (var (status, body) in cases)
        {
            var handler = new MockHttpHandler().Enqueue(status, body);
            using var client = new System.Net.Http.HttpClient(handler)
            {
                BaseAddress = new Uri("https://api.example.com")
            };

            var response = await client.GetAsync("/resource");
            Print.Line($"  Status: {(int)response.StatusCode} {response.StatusCode}");
            Print.Line($"  IsSuccess: {response.IsSuccessStatusCode}");

            if (!response.IsSuccessStatusCode)
            {
                var errorBody = await response.Content.ReadAsStringAsync();
                Print.Line($"  오류 본문: {errorBody}");
            }
        }

        // HttpRequestException 처리
        var faultHandler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.InternalServerError, "error");
        using var faultClient = new System.Net.Http.HttpClient(faultHandler)
        {
            BaseAddress = new Uri("https://api.example.com")
        };

        try
        {
            var r = await faultClient.GetAsync("/fail");
            r.EnsureSuccessStatusCode(); // 5xx → HttpRequestException throw
        }
        catch (HttpRequestException ex)
        {
            Print.Line($"  HttpRequestException: {ex.StatusCode} — {ex.Message[..Math.Min(50, ex.Message.Length)]}");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowHeaders()
    {
        Print.Section("1-4. 헤더 · 인증 · Base URL");

        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, """{"authenticated":true}""");

        using var client = new System.Net.Http.HttpClient(handler)
        {
            BaseAddress = new Uri("https://api.example.com")
        };

        // 기본 헤더
        client.DefaultRequestHeaders.Add("User-Agent", "DotNetPractice/1.0");
        client.DefaultRequestHeaders.Add("Accept", "application/json");
        client.DefaultRequestHeaders.Authorization =
            new System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", "eyJtoken...");

        var response = await client.GetAsync("/protected");
        Print.Line($"  인증 헤더 전송: {handler.SentRequests[0].Headers.Authorization?.Scheme} ***");
        Print.Line($"  응답: {response.StatusCode}");

        // 요청별 헤더
        handler.Enqueue(System.Net.HttpStatusCode.OK, """{}""");
        using var request = new HttpRequestMessage(HttpMethod.Get, "/custom");
        request.Headers.Add("X-Request-ID", Guid.NewGuid().ToString());
        request.Headers.Add("X-API-Version", "2024-06-01");
        var r2 = await client.SendAsync(request);
        Print.Line($"  커스텀 헤더 요청: {r2.StatusCode}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowDisposalPattern()
    {
        Print.Section("1-5. 올바른 HttpClient 사용 패턴");

        Print.Line("  NG: using var client = new HttpClient() — 포트 고갈 위험");
        Print.Line("     (소켓이 TIME_WAIT 상태로 남아 포트 고갈 유발)");
        Print.Line("");
        Print.Line("  OK1: static/singleton HttpClient — 기본 방법");
        Print.Line("  OK2: IHttpClientFactory (권장) — D2에서 다룸");
        Print.Line("  OK3: typed HttpClient via DI — D2에서 다룸");

        // 올바른 singleton 패턴
        Print.Line($"  Singleton 재사용: {HttpClientSingleton.Instance is not null}");
    }
}

static class HttpClientSingleton
{
    public static readonly System.Net.Http.HttpClient Instance = new()
    {
        BaseAddress = new Uri("https://api.example.com"),
        Timeout = TimeSpan.FromSeconds(30),
    };
}
