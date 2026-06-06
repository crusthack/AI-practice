namespace Async.Demos;

static class D2_AsyncAwait
{
    public static async Task Run()
    {
        Print.Header("2. async/await 패턴 심화");
        await ShowAsyncAwaitBasics();
        await ShowConfigureAwait();
        await ShowDeadlockAvoidance();
        await ShowAsyncBestPractices();
        await ShowAsyncVoid();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowAsyncAwaitBasics()
    {
        Print.Section("2-1. async/await 동작 원리");

        Print.Line($"  시작 스레드: {Environment.CurrentManagedThreadId}");

        await Task.Delay(1);  // 여기서 스레드 반환, 완료 후 재개

        Print.Line($"  재개 스레드: {Environment.CurrentManagedThreadId}  (같을 수도 다를 수도)");

        // async 메서드는 상태 머신으로 컴파일됨
        // - await 전: 동기적으로 실행
        // - await 후: 콜백으로 재개 (IO bound에서 스레드 해제)

        var result = await ComputeAsync(5);
        Print.Line($"  ComputeAsync(5): {result}");

        static async Task<int> ComputeAsync(int n)
        {
            await Task.Delay(1);    // 비동기 IO 시뮬레이션
            return n * n;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowConfigureAwait()
    {
        Print.Section("2-2. ConfigureAwait(false) — 컨텍스트 캡처 제어");

        // ConfigureAwait(true) = 기본값: 현재 SynchronizationContext에서 재개
        // ConfigureAwait(false): 임의 ThreadPool 스레드에서 재개 (라이브러리 코드 권장)

        await Task.Delay(1).ConfigureAwait(false);
        Print.Line($"  ConfigureAwait(false) 후 스레드: {Environment.CurrentManagedThreadId}");

        // 라이브러리 코드 패턴 — ConfigureAwait(false) 일관 사용
        var result = await LibraryMethodAsync().ConfigureAwait(false);
        Print.Line($"  라이브러리 결과: {result}");

        static async Task<string> LibraryMethodAsync()
        {
            await Task.Delay(1).ConfigureAwait(false);
            // 라이브러리 내부는 ConfigureAwait(false) 사용 → 컨텍스트 캡처 오버헤드 없음
            return "library_result";
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowDeadlockAvoidance()
    {
        Print.Section("2-3. 데드락 회피 패턴");

        // 안티패턴: .Result / .Wait() 를 동기 컨텍스트에서 쓰면 데드락 가능
        // NG (일부 환경): var r = GetDataAsync().Result;

        // 올바른 패턴 1: async all the way
        var data1 = await GetDataAsync();
        Print.Line($"  async all-the-way: {data1}");

        // 올바른 패턴 2: 부득이 동기 사용 시 ConfigureAwait(false) + .GetAwaiter().GetResult()
        // (단, 주 스레드가 SynchronizationContext 없을 때만)
        string data2 = GetDataAsync().ConfigureAwait(false).GetAwaiter().GetResult();
        Print.Line($"  GetAwaiter().GetResult(): {data2}");

        // 올바른 패턴 3: Task.Run 으로 컨텍스트 분리
        string data3 = await Task.Run(() => GetDataAsync());
        Print.Line($"  Task.Run wrap: {data3}");

        static async Task<string> GetDataAsync()
        {
            await Task.Delay(1).ConfigureAwait(false);
            return "data";
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowAsyncBestPractices()
    {
        Print.Section("2-4. 모범 사례 — 반환 타입 · 예외 전파");

        // Task 반환 vs void 반환
        // NG: async void (예외 잡을 수 없음, 테스트 불가)
        // OK: async Task (예외 전파 가능)

        // async 메서드에서 예외 — await 시 전파
        try
        {
            await FailingAsync();
        }
        catch (InvalidOperationException ex)
        {
            Print.Line($"  async 예외 전파: {ex.Message}");
        }

        // Task<bool> 반환으로 성공/실패 표현
        bool ok = await TryOperationAsync();
        Print.Line($"  TryOperation: {ok}");

        // 동기 완료 최적화 — if (!await 가능)
        var fastResult = await FastPath(useCache: true);
        Print.Line($"  FastPath (캐시): {fastResult}");

        static async Task FailingAsync()
        {
            await Task.Delay(1);
            throw new InvalidOperationException("비동기 예외");
        }

        static async Task<bool> TryOperationAsync()
        {
            try   { await Task.Delay(1); return true; }
            catch { return false; }
        }

        static ValueTask<string> FastPath(bool useCache)
        {
            if (useCache) return ValueTask.FromResult("cached");
            return new ValueTask<string>(FetchAsync());

            static async Task<string> FetchAsync()
            {
                await Task.Delay(10);
                return "fetched";
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowAsyncVoid()
    {
        Print.Section("2-5. async void — 이벤트 핸들러 한정 사용");

        // async void는 이벤트 핸들러에서만 허용
        // 예외는 AppDomain.UnhandledException으로 전파됨
        bool eventFired = false;
        var cts = new CancellationTokenSource(200);

        // 시뮬레이션: 이벤트 핸들러 대신 await로 래핑
        async Task SimulateEventHandler()
        {
            await Task.Delay(1);
            eventFired = true;
        }

        await SimulateEventHandler();
        Print.Line($"  이벤트 핸들러 완료: {eventFired}");

        // async void 올바른 패턴: 내부에서 예외 처리
        AsyncVoidHandler(ok: true);
        await Task.Delay(20); // 핸들러 완료 대기

        static async void AsyncVoidHandler(bool ok)
        {
            try
            {
                await Task.Delay(1);
                Console.WriteLine($"  async void 완료 (ok={ok})");
            }
            catch (Exception ex)
            {
                // async void에서는 예외를 반드시 여기서 잡아야 함
                Console.WriteLine($"  async void 예외: {ex.Message}");
            }
        }
    }
}
