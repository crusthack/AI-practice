using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using System.Text.Json;

namespace HttpClientDemo.Demos;

// ── Typed HttpClient ──────────────────────────────────────────────────────────

public class GitHubApiClient(System.Net.Http.HttpClient httpClient)
{
    private static readonly JsonSerializerOptions Opts = new() { PropertyNamingPolicy = JsonNamingPolicy.SnakeCaseLower };

    public async Task<GitHubUser?> GetUserAsync(string login, CancellationToken ct = default)
    {
        var response = await httpClient.GetAsync($"/users/{login}", ct);
        if (response.StatusCode == System.Net.HttpStatusCode.NotFound) return null;
        response.EnsureSuccessStatusCode();
        return await response.Content.ReadFromJsonAsync<GitHubUser>(Opts, ct);
    }
}

public class WeatherApiClient(System.Net.Http.HttpClient httpClient)
{
    public async Task<string> GetForecastAsync(string city)
    {
        var response = await httpClient.GetAsync($"/forecast/{city}");
        response.EnsureSuccessStatusCode();
        return await response.Content.ReadAsStringAsync();
    }
}

// ── Delegating Handler ────────────────────────────────────────────────────────

public class LoggingHandler(ILogger<LoggingHandler> logger) : DelegatingHandler
{
    protected override async Task<HttpResponseMessage> SendAsync(
        HttpRequestMessage request, CancellationToken ct)
    {
        var sw = System.Diagnostics.Stopwatch.StartNew();
        logger.LogInformation("→ {Method} {Uri}", request.Method, request.RequestUri);

        HttpResponseMessage response = await base.SendAsync(request, ct);

        sw.Stop();
        logger.LogInformation("← {StatusCode} ({ElapsedMs}ms)", response.StatusCode, sw.ElapsedMilliseconds);
        return response;
    }
}

public class AuthHandler(string apiKey) : DelegatingHandler
{
    protected override Task<HttpResponseMessage> SendAsync(
        HttpRequestMessage request, CancellationToken ct)
    {
        request.Headers.Add("X-Api-Key", apiKey);
        return base.SendAsync(request, ct);
    }
}

// ── 데모 ──────────────────────────────────────────────────────────────────────

static class D2_IHttpClientFactory
{
    public static async Task Run()
    {
        Print.Header("2. IHttpClientFactory — Named · Typed · DelegatingHandler");
        await ShowNamedClient();
        await ShowTypedClient();
        await ShowDelegatingHandler();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowNamedClient()
    {
        Print.Section("2-1. Named HttpClient");

        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, """{"login":"octocat","name":"The Octocat","public_repos":8}""");

        var services = new ServiceCollection();
        services.AddHttpClient("github", client =>
        {
            client.BaseAddress = new Uri("https://api.github.com");
            client.DefaultRequestHeaders.Add("User-Agent", "DotNetPractice/1.0");
        }).ConfigurePrimaryHttpMessageHandler(() => handler);

        using var sp = services.BuildServiceProvider();
        var factory = sp.GetRequiredService<IHttpClientFactory>();

        var client = factory.CreateClient("github");
        var response = await client.GetAsync("/users/octocat");
        var content  = await response.Content.ReadAsStringAsync();
        Print.Line($"Named client: {response.StatusCode}  body: {content[..Math.Min(40, content.Length)]}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowTypedClient()
    {
        Print.Section("2-2. Typed HttpClient");

        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, """{"login":"testuser","name":"Test User","public_repos":5}""");

        var services = new ServiceCollection();
        services.AddLogging();
        services.AddHttpClient<GitHubApiClient>(client =>
        {
            client.BaseAddress = new Uri("https://api.github.com");
        }).ConfigurePrimaryHttpMessageHandler(() => handler);

        using var sp = services.BuildServiceProvider();
        var apiClient = sp.GetRequiredService<GitHubApiClient>();

        var user = await apiClient.GetUserAsync("testuser");
        Print.Line($"Typed client: {user?.Login}  repos={user?.PublicRepos}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowDelegatingHandler()
    {
        Print.Section("2-3. DelegatingHandler — 미들웨어 파이프라인");

        var innerHandler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, """{"data":"success"}""");

        var services = new ServiceCollection();
        services.AddLogging(b => b.SetMinimumLevel(LogLevel.Information));
        services.AddTransient<LoggingHandler>();
        services.AddHttpClient("decorated", c => c.BaseAddress = new Uri("https://api.example.com"))
                .ConfigurePrimaryHttpMessageHandler(() => innerHandler)
                .AddHttpMessageHandler<LoggingHandler>();

        using var sp = services.BuildServiceProvider();
        var factory = sp.GetRequiredService<IHttpClientFactory>();
        var client  = factory.CreateClient("decorated");

        var response = await client.GetAsync("/data");
        Print.Line($"DelegatingHandler 체인: {response.StatusCode}");

        // 요청 수 검증
        Print.Line($"  요청 수: {innerHandler.SentRequests.Count}");
    }
}
