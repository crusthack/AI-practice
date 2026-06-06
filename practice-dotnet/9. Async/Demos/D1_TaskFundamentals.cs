namespace Async.Demos;

static class D1_TaskFundamentals
{
    public static async Task Run()
    {
        Print.Header("1. Task 기초 — 생성 · 조합 · 예외");
        await ShowTaskCreation();
        await ShowTaskCombinators();
        await ShowTaskException();
        ShowTaskStatus();
        await ShowContinuationWith();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowTaskCreation()
    {
        Print.Section("1-1. Task 생성 방식 비교");

        // Task.Run — ThreadPool 실행
        var t1 = Task.Run(() => { Thread.Sleep(10); return 42; });
        Print.Line($"Task.Run:            {await t1}");

        // Task.FromResult — 이미 완료된 값 래핑
        var t2 = Task.FromResult("immediate");
        Print.Line($"Task.FromResult:     {await t2}  (할당 최소화)");

        // Task.CompletedTask — void async 반환용
        await Task.CompletedTask;
        Print.Line($"Task.CompletedTask:  IsCompleted={Task.CompletedTask.IsCompleted}");

        // Task.Delay — 비동기 대기 (Thread.Sleep 아님)
        var sw = System.Diagnostics.Stopwatch.StartNew();
        await Task.Delay(20);
        Print.Line($"Task.Delay(20):      {sw.ElapsedMilliseconds}ms");

        // TaskCompletionSource — 외부에서 완료 제어
        var tcs = new TaskCompletionSource<int>();
        _ = Task.Run(async () => { await Task.Delay(5); tcs.SetResult(99); });
        Print.Line($"TCS:                 {await tcs.Task}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowTaskCombinators()
    {
        Print.Section("1-2. Task.WhenAll · WhenAny · WhenEach");

        // WhenAll — 모두 완료 대기
        var tasks = Enumerable.Range(1, 4)
            .Select(i => Task.Run(async () => { await Task.Delay(i * 10); return i * i; }))
            .ToArray();

        var sw = System.Diagnostics.Stopwatch.StartNew();
        int[] results = await Task.WhenAll(tasks);
        Print.Line($"WhenAll ({sw.ElapsedMilliseconds}ms): [{string.Join(", ", results)}]");

        // WhenAny — 가장 먼저 완료된 것
        var raceTasks = Enumerable.Range(1, 3)
            .Select(i => Task.Delay(i * 50).ContinueWith(_ => i))
            .ToArray();
        var first = await await Task.WhenAny(raceTasks);
        Print.Line($"WhenAny: 가장 빨리 완료: {first}번 태스크");

        // Task.WhenEach (.NET 9+) — 완료 순서대로 소비
        var weTasks = Enumerable.Range(1, 3)
            .Select(i => Task.Run(async () => { await Task.Delay((4 - i) * 20); return i; }));
        var order = new List<int>();
        await foreach (var t in Task.WhenEach(weTasks))
            order.Add(await t);
        Print.Line($"WhenEach 완료 순서: [{string.Join(", ", order)}]");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowTaskException()
    {
        Print.Section("1-3. Task 예외 처리");

        // 단일 Task 예외
        try
        {
            await Task.Run(() => throw new InvalidOperationException("Task 실패"));
        }
        catch (InvalidOperationException ex)
        {
            Print.Line($"단일 예외:  {ex.Message}");
        }

        // WhenAll 예외 — 첫 번째만 unwrap
        var faulted = new Task[]
        {
            Task.Run(() => throw new IOException("IO 오류")),
            Task.Run(() => throw new ArgumentException("Arg 오류")),
        };
        try
        {
            await Task.WhenAll(faulted);
        }
        catch
        {
            var all = faulted
                .Where(t => t.IsFaulted)
                .SelectMany(t => t.Exception!.InnerExceptions)
                .ToList();
            Print.Line($"WhenAll 전체 예외 수: {all.Count}");
            foreach (var ex in all)
                Print.Line($"  • {ex.GetType().Name}: {ex.Message}");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowTaskStatus()
    {
        Print.Section("1-4. Task 상태 — Status · IsCompleted · IsFaulted");

        var created  = new Task(() => { });
        var running  = Task.Run(() => Thread.Sleep(200));
        var done     = Task.FromResult(1);
        var canceled = Task.FromCanceled(new CancellationToken(canceled: true));
        var faulted  = Task.FromException(new Exception("error"));

        Thread.Sleep(10);
        Print.Line($"Created:   {created.Status}");
        Print.Line($"Running:   {running.Status}");
        Print.Line($"Done:      {done.Status}   IsCompleted={done.IsCompleted}");
        Print.Line($"Canceled:  {canceled.Status}  IsCanceled={canceled.IsCanceled}");
        Print.Line($"Faulted:   {faulted.Status}   IsFaulted={faulted.IsFaulted}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowContinuationWith()
    {
        Print.Section("1-5. ContinueWith — 연속 실행 체인");

        var result = await Task.Run(() => 10)
            .ContinueWith(t => t.Result * 2,
                TaskContinuationOptions.OnlyOnRanToCompletion)
            .ContinueWith(t => $"최종: {t.Result}",
                TaskContinuationOptions.OnlyOnRanToCompletion);

        Print.Line($"ContinueWith 체인: {result}");

        // 항상 실행 (정리용)
        await Task.Run(() => 99)
            .ContinueWith(t => Print.Line($"  후처리 (항상): result={t.Result}"),
                TaskContinuationOptions.None);
    }
}
