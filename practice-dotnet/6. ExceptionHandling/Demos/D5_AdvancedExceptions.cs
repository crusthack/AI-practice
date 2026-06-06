using System.Runtime.ExceptionServices;

namespace ExceptionHandling.Demos;

static class D5_AdvancedExceptions
{
    public static async Task Run()
    {
        Print.Header("5. 고급 예외 처리");
        ShowAggregateException();
        await ShowAggregateFromTasksAsync();
        ShowExceptionDispatchInfo();
        ShowObjectDisposedPattern();
        await ShowAsyncFinallyAsync();
        ShowUnobservedTaskException();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowAggregateException()
    {
        Print.Section("5-1. AggregateException — Parallel / PLINQ");

        try
        {
            Parallel.For(0, 5, i =>
            {
                if (i % 2 == 0) throw new InvalidOperationException($"짝수 실패: {i}");
            });
        }
        catch (AggregateException ae)
        {
            Print.Line($"  AggregateException 내 예외 수: {ae.InnerExceptions.Count}");
            // Flatten: 중첩된 AggregateException 평탄화
            foreach (var ex in ae.Flatten().InnerExceptions)
                Print.Line($"    • {ex.GetType().Name}: {ex.Message}");
        }

        // Handle: 처리 가능한 예외만 소비, 나머지는 다시 throw
        try
        {
            var ae = new AggregateException(
                new InvalidOperationException("복구 가능"),
                new OutOfMemoryException("치명적"));

            ae.Handle(ex =>
            {
                if (ex is InvalidOperationException) { Print.Line($"  처리됨: {ex.Message}"); return true; }
                return false; // 처리 못 함 → 재throw
            });
        }
        catch (AggregateException ae2)
        {
            Print.Line($"  미처리 잔여: {ae2.InnerExceptions[0].GetType().Name}");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowAggregateFromTasksAsync()
    {
        Print.Section("5-2. Task.WhenAll — 다중 실패 집약");

        var tasks = new Task[]
        {
            Task.Run(() => throw new IOException("파일 없음")),
            Task.Run(() => throw new TimeoutException("타임아웃")),
            Task.Run(() => { /* 성공 */ }),
        };

        try { await Task.WhenAll(tasks); }
        catch (Exception ex) when (tasks.Any(t => t.IsFaulted))
        {
            // await는 첫 번째 예외만 unwrap — 전체 보려면 Task.Exception 직접 확인
            Print.Line($"  await 후 받은 예외: {ex.GetType().Name}");
            var allExceptions = tasks
                .Where(t => t.IsFaulted)
                .SelectMany(t => t.Exception!.InnerExceptions);
            foreach (var e in allExceptions)
                Print.Line($"    • {e.GetType().Name}: {e.Message}");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowExceptionDispatchInfo()
    {
        Print.Section("5-3. ExceptionDispatchInfo — 스택 추적 보존 재throw");

        ExceptionDispatchInfo? captured = null;

        // 예외를 나중에 다른 스레드/문맥에서 재throw해야 할 때
        try
        {
            try { ThrowDeep(); }
            catch (Exception ex)
            {
                // 원래 스택 추적 포함하여 보존
                captured = ExceptionDispatchInfo.Capture(ex);
                Print.Line($"  예외 캡처됨: {ex.Message}");
                Print.Line($"  스택 줄 수: {ex.StackTrace?.Split('\n').Length}");
            }
        }
        catch { }

        // 나중에 원래 스택 추적을 유지하며 재throw
        try
        {
            captured?.Throw(); // throw 지점이 아닌 원래 발생 지점 스택 유지
        }
        catch (Exception ex)
        {
            Print.Line($"  재throw 후 스택 줄 수: {ex.StackTrace?.Split('\n').Length}  ← 동일");
        }

        static void ThrowDeep() => DeepCall();
        static void DeepCall() => throw new InvalidOperationException("깊은 호출 예외");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowObjectDisposedPattern()
    {
        Print.Section("5-4. ObjectDisposedException 패턴");

        var resource = new ManagedResource();
        resource.DoWork();
        resource.Dispose();

        try { resource.DoWork(); }
        catch (ObjectDisposedException ex)
        {
            Print.Line($"  ObjectDisposed: {ex.ObjectName}");
        }

        // using 패턴 — 자동 Dispose
        using (var r2 = new ManagedResource())
        {
            r2.DoWork();
        } // Dispose 자동 호출

        Print.Line($"  using 블록 후 disposed: {true}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowAsyncFinallyAsync()
    {
        Print.Section("5-5. async/finally — 취소와 예외");

        var cts = new CancellationTokenSource();

        try
        {
            await DoAsyncWork(cts, cts.Token);
        }
        catch (OperationCanceledException)
        {
            Print.Line("  작업 취소됨");
        }
        finally
        {
            Print.Line("  finally 실행 (취소/예외/정상 모두)");
            cts.Dispose();
        }

        // ConfigureAwait(false) 후에도 finally는 실행됨
        try
        {
            await Task.Run(() => throw new InvalidOperationException("async 예외"))
                      .ConfigureAwait(false);
        }
        catch (InvalidOperationException ex)
        {
            Print.Line($"  ConfigureAwait 후 catch: {ex.Message}");
        }

        static async Task DoAsyncWork(CancellationTokenSource src, CancellationToken ct)
        {
            await Task.Delay(1, ct);
            src.Cancel(); // 즉시 취소
            await Task.Delay(1000, ct); // 취소됨
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowUnobservedTaskException()
    {
        Print.Section("5-6. TaskScheduler.UnobservedTaskException");

        bool caught = false;
        EventHandler<UnobservedTaskExceptionEventArgs> handler = (_, e) =>
        {
            caught = true;
            Print.Line($"  UnobservedTask: {e.Exception.InnerExceptions[0].Message}");
            e.SetObserved(); // 프로세스 크래시 방지
        };

        TaskScheduler.UnobservedTaskException += handler;
        try
        {
            // 미관찰 Task 예외 생성
            _ = Task.Run(() => throw new InvalidOperationException("미관찰 예외"));
            Thread.Sleep(50);
            GC.Collect();
            GC.WaitForPendingFinalizers();
            Thread.Sleep(50);
            Print.Line($"  UnobservedTaskException 발생: {caught}");
        }
        finally
        {
            TaskScheduler.UnobservedTaskException -= handler;
        }
    }
}

// ── 헬퍼 클래스 ───────────────────────────────────────────────────────────────

sealed class ManagedResource : IDisposable
{
    private bool _disposed;

    public void DoWork()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        Console.WriteLine("  DoWork 실행");
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        Console.WriteLine("  ManagedResource.Dispose()");
    }
}
