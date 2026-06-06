namespace HttpClientDemo.Demos;

// ── 테스트/데모용 Mock HTTP 핸들러 ────────────────────────────────────────────

public class MockHttpHandler : HttpMessageHandler
{
    private readonly Queue<HttpResponseMessage> _responses = new();
    public List<HttpRequestMessage> SentRequests { get; } = [];

    public MockHttpHandler Enqueue(HttpStatusCode status, string content,
        string contentType = "application/json")
    {
        _responses.Enqueue(new HttpResponseMessage(status)
        {
            Content = new StringContent(content, System.Text.Encoding.UTF8, contentType)
        });
        return this;
    }

    protected override Task<HttpResponseMessage> SendAsync(
        HttpRequestMessage request, CancellationToken ct)
    {
        SentRequests.Add(request);
        if (_responses.TryDequeue(out var response))
            return Task.FromResult(response);
        return Task.FromResult(new HttpResponseMessage(HttpStatusCode.InternalServerError));
    }
}

public static class HttpStatusCode
{
    public const System.Net.HttpStatusCode OK                  = System.Net.HttpStatusCode.OK;
    public const System.Net.HttpStatusCode Created             = System.Net.HttpStatusCode.Created;
    public const System.Net.HttpStatusCode BadRequest          = System.Net.HttpStatusCode.BadRequest;
    public const System.Net.HttpStatusCode NotFound            = System.Net.HttpStatusCode.NotFound;
    public const System.Net.HttpStatusCode InternalServerError = System.Net.HttpStatusCode.InternalServerError;
    public const System.Net.HttpStatusCode TooManyRequests     = System.Net.HttpStatusCode.TooManyRequests;
    public const System.Net.HttpStatusCode ServiceUnavailable  = System.Net.HttpStatusCode.ServiceUnavailable;
}
