namespace Async.Demos;

static class D4_Cancellation
{
    public static async Task Run()
    {
        Print.Header("4. CancellationToken — 취소 패턴");
        await ShowBasicCancellation();
        await ShowLinkedCancellation();
        await ShowCancellationCallbacks();
        await ShowGracefulShutdown();
        ShowCancellationBestPractices();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowBasicCancellation()
    {
        Print.Section("4-1. CancellationTokenSource 기본");

        using var cts = new CancellationTokenSource();
        var token = cts.Token;

        // 취소 트리거
        _ = Task.Run(async () => { await Task.Delay(30); cts.Cancel(); });

        try
        {
            var count = 0;
            while (!token.IsCancellationRequested)
            {
                await Task.Delay(10, token);
                count++;
            }
        }
        catch (OperationCanceledException)
        {
            Print.Line($"  취소됨 (IsCancellationRequested={token.IsCancellationRequested})");
        }

        // 시간 기반 취소
        using var timeCts = new CancellationTokenSource(TimeSpan.FromMilliseconds(50));
        try
        {
            await Task.Delay(200, timeCts.Token);
        }
        catch (OperationCanceledException)
        {
            Print.Line($"  시간 초과 취소 (50ms)");
        }

        // ThrowIfCancellationRequested
        using var cts2 = new CancellationTokenSource();
        cts2.Cancel();
        try
        {
            cts2.Token.ThrowIfCancellationRequested();
        }
        catch (OperationCanceledException)
        {
            Print.Line($"  ThrowIfCancellationRequested: 동작 확인");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowLinkedCancellation()
    {
        Print.Section("4-2. LinkedTokenSource — 취소 연결");

        using var parent  = new CancellationTokenSource();
        using var timeout = new CancellationTokenSource(100);
        using var linked  = CancellationTokenSource.CreateLinkedTokenSource(
            parent.Token, timeout.Token);

        Print.Line($"  linked.IsCancellationRequested (초기): {linked.Token.IsCancellationRequested}");

        // timeout이 먼저 취소되면 linked도 취소됨
        try
        {
            await Task.Delay(200, linked.Token);
        }
        catch (OperationCanceledException)
        {
            Print.Line($"  linked 취소: timeout={timeout.IsCancellationRequested}  parent={parent.IsCancellationRequested}");
        }

        // 부모를 취소하면 자식(linked)도 취소
        using var parent2 = new CancellationTokenSource();
        using var child   = CancellationTokenSource.CreateLinkedTokenSource(parent2.Token);
        parent2.Cancel();
        Print.Line($"  부모 취소 → 자식: {child.Token.IsCancellationRequested}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowCancellationCallbacks()
    {
        Print.Section("4-3. 취소 콜백 등록");

        using var cts = new CancellationTokenSource();

        var log = new List<string>();

        // Register: 취소 시 콜백 실행
        using var reg1 = cts.Token.Register(() => log.Add("콜백 1"));
        using var reg2 = cts.Token.Register(() => log.Add("콜백 2"));
        // LIFO 순서로 실행됨

        await Task.Delay(10);
        cts.Cancel();
        await Task.Delay(10); // 콜백 완료 대기

        Print.Line($"  콜백 실행 순서: {string.Join(", ", log)}  (역순 LIFO)");

        // Register 해제
        using var cts2 = new CancellationTokenSource();
        CancellationTokenRegistration registration = cts2.Token.Register(() => log.Add("미해제 콜백"));
        registration.Dispose(); // 등록 해제
        cts2.Cancel();
        Print.Line($"  해제된 콜백 호출 안 됨: {!log.Contains("미해제 콜백")}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowGracefulShutdown()
    {
        Print.Section("4-4. Graceful Shutdown 패턴");

        using var cts = new CancellationTokenSource();
        var results = new System.Collections.Concurrent.ConcurrentBag<int>();

        // 작업 풀 — 취소 신호를 받으면 우아하게 종료
        var workers = Enumerable.Range(0, 3)
            .Select(id => Worker(id, cts.Token, results))
            .ToArray();

        await Task.Delay(50);
        cts.Cancel(); // 종료 신호

        try { await Task.WhenAll(workers); }
        catch (OperationCanceledException) { /* 예상된 취소 */ }

        Print.Line($"  처리된 작업 수: {results.Count}개  (취소 전까지)");

        static async Task Worker(int id, CancellationToken ct, System.Collections.Concurrent.ConcurrentBag<int> results)
        {
            int count = 0;
            while (!ct.IsCancellationRequested)
            {
                await Task.Delay(15, ct);
                results.Add(id * 100 + count++);
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowCancellationBestPractices()
    {
        Print.Section("4-5. 모범 사례 요약");

        Print.Line("  1. CancellationToken은 매개변수 마지막에, 기본값 default");
        Print.Line("  2. Task.Delay/IO API에 token 전달 → 즉시 응답");
        Print.Line("  3. CPU-bound 루프: ct.IsCancellationRequested 주기적 확인");
        Print.Line("  4. OperationCanceledException을 catch → 정리 후 재throw 또는 무시");
        Print.Line("  5. CancellationTokenSource 는 using으로 Dispose");
        Print.Line("  6. linked token: 타임아웃 + 부모 취소 조합 시 유용");
        Print.Line("  7. ThrowIfCancellationRequested: 루프 진입 전에 체크");
    }
}
