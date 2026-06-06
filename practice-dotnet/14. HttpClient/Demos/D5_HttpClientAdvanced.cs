using System.Net.Http.Json;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.Json;

namespace HttpClientDemo.Demos;

static class D5_HttpClientAdvanced
{
    public static async Task Run()
    {
        Print.Header("5. 고급 패턴 — 스트리밍 · Multipart · 진행률 · 병렬 요청");
        await ShowStreamingDownload();
        await ShowStreamingJson();
        await ShowMultipartUpload();
        await ShowParallelRequests();
        ShowCancellationPattern();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowStreamingDownload()
    {
        Print.Section("5-1. 스트리밍 다운로드 — ResponseHeadersRead");

        const string largeBody = "Line1\nLine2\nLine3\nLine4\nLine5\n";
        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, largeBody, "text/plain");

        using var client = new System.Net.Http.HttpClient(handler)
        {
            BaseAddress = new Uri("https://files.example.com")
        };

        // HttpCompletionOption.ResponseHeadersRead → 헤더 수신 직후 반환 (body 미버퍼링)
        var response = await client.GetAsync("/large-file", HttpCompletionOption.ResponseHeadersRead);
        response.EnsureSuccessStatusCode();

        var lineCount = 0;
        using var stream = await response.Content.ReadAsStreamAsync();
        using var reader = new StreamReader(stream);
        while (await reader.ReadLineAsync() is { } line)
        {
            lineCount++;
        }
        Print.Line($"  스트리밍으로 읽은 줄 수: {lineCount}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowStreamingJson()
    {
        Print.Section("5-2. Server-Sent Events (JSON 스트림) 시뮬레이션");

        // JSON Lines 형식 (줄마다 JSON 객체)
        var jsonLines = string.Join("\n", Enumerable.Range(1, 5)
            .Select(i => JsonSerializer.Serialize(new { id = i, value = $"item_{i}" })));
        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, jsonLines, "application/x-ndjson");

        using var client = new System.Net.Http.HttpClient(handler)
        {
            BaseAddress = new Uri("https://stream.example.com")
        };

        var items = ReadJsonStreamAsync(client, "/stream");
        var count = 0;
        await foreach (var item in items)
        {
            count++;
        }
        Print.Line($"  스트리밍 JSON 아이템 수신: {count}개");
    }

    static async IAsyncEnumerable<JsonElement> ReadJsonStreamAsync(
        System.Net.Http.HttpClient client, string path,
        [EnumeratorCancellation] CancellationToken ct = default)
    {
        var response = await client.GetAsync(path, HttpCompletionOption.ResponseHeadersRead, ct);
        response.EnsureSuccessStatusCode();
        using var stream = await response.Content.ReadAsStreamAsync(ct);
        using var reader = new StreamReader(stream);

        while (!reader.EndOfStream && !ct.IsCancellationRequested)
        {
            var line = await reader.ReadLineAsync(ct);
            if (string.IsNullOrEmpty(line)) continue;
            yield return JsonSerializer.Deserialize<JsonElement>(line);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowMultipartUpload()
    {
        Print.Section("5-3. Multipart/Form-Data 파일 업로드");

        var handler = new MockHttpHandler()
            .Enqueue(System.Net.HttpStatusCode.OK, """{"uploaded":"photo.jpg","size":1024}""");

        using var client = new System.Net.Http.HttpClient(handler)
        {
            BaseAddress = new Uri("https://upload.example.com")
        };

        // 파일 콘텐츠 시뮬레이션
        var fileBytes = Encoding.UTF8.GetBytes("FAKE_JPEG_CONTENT_1234");
        using var multipart = new MultipartFormDataContent();

        // 파일 파트
        var filePart = new ByteArrayContent(fileBytes);
        filePart.Headers.ContentType = new System.Net.Http.Headers.MediaTypeHeaderValue("image/jpeg");
        multipart.Add(filePart, "file", "photo.jpg");

        // 메타데이터 파트
        multipart.Add(new StringContent("profile"), "category");
        multipart.Add(new StringContent("true"), "public");

        var response = await client.PostAsync("/upload", multipart);
        response.EnsureSuccessStatusCode();
        var result = await response.Content.ReadAsStringAsync();
        Print.Line($"  업로드 결과: {result}");

        // 실제 전송된 Content-Type 확인
        var sentContentType = handler.SentRequests[0].Content?.Headers.ContentType?.MediaType;
        Print.Line($"  전송 Content-Type: {sentContentType}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowParallelRequests()
    {
        Print.Section("5-4. 병렬 요청 — Task.WhenAll · 세마포어 제한");

        var handler = new MockHttpHandler();
        for (int i = 0; i < 5; i++)
            handler.Enqueue(System.Net.HttpStatusCode.OK, $"""{{\"id\":{i}}}""");

        using var client = new System.Net.Http.HttpClient(handler)
        {
            BaseAddress = new Uri("https://api.example.com")
        };

        // 병렬 요청 (제한 없음)
        var tasks = Enumerable.Range(0, 5)
            .Select(i => client.GetAsync($"/item/{i}"));
        var responses = await Task.WhenAll(tasks);
        Print.Line($"  병렬 요청 5개 완료: 성공={responses.Count(r => r.IsSuccessStatusCode)}개");

        // 세마포어로 동시 요청 수 제한
        var semaphore = new SemaphoreSlim(2); // 최대 2개 동시 요청
        for (int i = 0; i < 5; i++)
            handler.Enqueue(System.Net.HttpStatusCode.OK, $"""{{\"id\":{i}}}""");

        var throttledTasks = Enumerable.Range(0, 5).Select(async i =>
        {
            await semaphore.WaitAsync();
            try { return await client.GetAsync($"/item/{i}"); }
            finally { semaphore.Release(); }
        });

        var throttledResults = await Task.WhenAll(throttledTasks);
        Print.Line($"  세마포어 제한(2) 병렬 요청: 성공={throttledResults.Count(r => r.IsSuccessStatusCode)}개");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowCancellationPattern()
    {
        Print.Section("5-5. 취소 패턴 정리");

        Print.Line("  1. CancellationToken 전파: GetAsync(uri, ct) 항상 ct 전달");
        Print.Line("  2. Timeout: HttpClient.Timeout 또는 CancellationTokenSource(ms)");
        Print.Line("  3. 사용자 취소: LinkedTokenSource로 timeout + user cancel 합성");
        Print.Line("  4. 재시도 시 새 요청 생성: HttpRequestMessage는 1회용 (재사용 불가)");
        Print.Line("  5. ConfigureAwait(false): 라이브러리 코드에서 컨텍스트 캡처 방지");
    }
}
