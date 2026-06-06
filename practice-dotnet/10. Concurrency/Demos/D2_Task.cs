namespace Demos;

// ───────────────────────────────────────────────────────
// Task & async/await
//
// Task<T>      : 비동기 작업의 표현. 결과·예외·취소 상태를 담는다.
// async/await  : Task 체인을 동기 코드처럼 읽히게 해주는 문법 설탕.
//                컴파일러가 상태 머신으로 변환 → 스레드 블록 없음.
// ValueTask<T> : 동기 완료 경로에서 힙 할당을 절감. 캐시 계층에 적합.
// ───────────────────────────────────────────────────────
static class D2_Task
{
    public static async Task RunAsync()
    {
        Print.Header("2. Task & async/await");

        // ── 2-1. Task.Run — CPU 바운드를 ThreadPool로 오프로드
        Print.Section("2-1. Task.Run — CPU 바운드 작업 오프로드");
        {
            int result = await Task.Run(() =>
            {
                int sum = 0;
                for (int i = 0; i < 1_000_000; i++) sum += i;
                return sum;
            });
            Console.WriteLine($"    합계: {result:N0}  (ThreadPool 스레드에서 계산)");
        }

        // ── 2-2. Task.WhenAll — 여러 작업을 동시에 시작하고 모두 기다림
        Print.Section("2-2. Task.WhenAll — 동시 실행 후 전부 완료 대기");
        {
            var sw = System.Diagnostics.Stopwatch.StartNew();
            await Task.WhenAll(
                Task.Delay(100),
                Task.Delay(100),
                Task.Delay(100)
            );
            Console.WriteLine($"    3개 × 100ms 동시 실행: {sw.ElapsedMilliseconds}ms (순차면 ~300ms)");
        }

        // ── 2-3. Task.WhenAny — 첫 번째 완료만 기다림 (타임아웃 패턴)
        Print.Section("2-3. Task.WhenAny — 타임아웃 패턴");
        {
            var sw = System.Diagnostics.Stopwatch.StartNew();
            Task work    = Task.Delay(500);
            Task timeout = Task.Delay(150);
            Task first   = await Task.WhenAny(work, timeout);
            Console.WriteLine(first == timeout
                ? $"    타임아웃 발생! ({sw.ElapsedMilliseconds}ms)"
                : $"    작업 완료 ({sw.ElapsedMilliseconds}ms)");
        }

        // ── 2-4. CancellationToken — 협조적 취소
        Print.Section("2-4. CancellationToken — 협조적 취소 (Cooperative Cancellation)");
        {
            using var cts = new CancellationTokenSource(TimeSpan.FromMilliseconds(180));
            try
            {
                for (int i = 0; i < 10; i++)
                {
                    cts.Token.ThrowIfCancellationRequested(); // 취소 확인
                    await Task.Delay(60, cts.Token);           // 딜레이 중에도 취소 감지
                    Console.WriteLine($"    이터레이션 {i} 완료");
                }
            }
            catch (OperationCanceledException)
            {
                Console.WriteLine("    → OperationCanceledException: 협조적으로 취소됨");
            }
        }

        // ── 2-5. CancellationTokenSource 연결 (Linked)
        Print.Section("2-5. CancellationTokenSource.CreateLinkedTokenSource");
        {
            using var userCts    = new CancellationTokenSource();
            using var timeoutCts = new CancellationTokenSource(TimeSpan.FromMilliseconds(50));
            using var linked     = CancellationTokenSource.CreateLinkedTokenSource(
                                       userCts.Token, timeoutCts.Token);
            try
            {
                // 둘 중 어느 쪽이든 취소되면 linked도 취소됨
                await Task.Delay(200, linked.Token);
            }
            catch (OperationCanceledException)
            {
                Console.WriteLine($"    타임아웃에 의해 취소됨: {timeoutCts.IsCancellationRequested}");
            }
        }

        // ── 2-6. ValueTask — 동기 완료 경로에서 힙 할당 제거
        Print.Section("2-6. ValueTask<T> — 캐시 히트 시 할당 없음");
        {
            // 캐시에 있으면 ValueTask.FromResult → 힙 할당 없음
            // 없으면 비동기 DB/네트워크 → Task 할당 발생
            int hit  = await ReadValueAsync(cached: true);
            int miss = await ReadValueAsync(cached: false);
            Console.WriteLine($"    캐시 히트(동기): {hit}  |  캐시 미스(비동기): {miss}");
            Console.WriteLine("    Task 반환: 항상 힙 객체 할당");
            Console.WriteLine("    ValueTask : 동기 경로에서 스택 값만 사용 → 할당 없음");
        }

        // ── 2-7. ConfigureAwait(false)
        Print.Section("2-7. ConfigureAwait(false) — 동기화 컨텍스트 복귀 생략");
        Console.WriteLine("    await 뒤 코드는 기본적으로 원래 SynchronizationContext로 복귀");
        Console.WriteLine("    ConfigureAwait(false): 복귀하지 않음 → UI/ASP.NET 스레드 재점유 안 함");
        Console.WriteLine("    라이브러리 코드: ConfigureAwait(false) 권장 (데드락 방지 + 성능)");
        Console.WriteLine("    앱 코드 (UI): UI 요소 접근이 필요하면 ConfigureAwait(true) 유지");
        await Task.Delay(1).ConfigureAwait(false); // 예시
    }

    static readonly int _cachedValue = 42;

    static ValueTask<int> ReadValueAsync(bool cached)
    {
        if (cached)
            return ValueTask.FromResult(_cachedValue); // 할당 없음
        return new ValueTask<int>(FetchFromDbAsync());
    }

    static async Task<int> FetchFromDbAsync()
    {
        await Task.Delay(1); // DB/네트워크 시뮬레이션
        return 99;
    }
}
