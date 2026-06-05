namespace Demos;

// ───────────────────────────────────────────────────────
// 동기화 원시 타입 (Synchronization Primitives)
//
// 공유 상태를 여러 스레드가 안전하게 접근하기 위한 도구들.
//
// lock (Monitor)      : 가장 기본. 상호 배제(mutex). OS 커널 진입 가능.
// Interlocked         : CPU 원자 명령어(LOCK prefix). 가장 가볍다.
// SpinLock            : 커널 진입 없이 CPU 루프로 대기. 짧은 임계구역 전용.
// SemaphoreSlim       : N개 동시 진입 허용. async 지원(WaitAsync).
// ReaderWriterLockSlim: 읽기는 동시, 쓰기만 배타적.
// ManualResetEventSlim: 게이트 패턴 — 모든 대기자를 한 번에 통과.
// CountdownEvent      : N개 신호를 모은 뒤 대기자 해제.
// Barrier             : 모든 참가자가 특정 지점 도달 시 다음 단계 진행.
// ───────────────────────────────────────────────────────
static class D4_Sync
{
    public static async Task RunAsync()
    {
        Print.Header("4. 동기화 원시 타입 (Synchronization Primitives)");

        // ── 4-1. lock (Monitor)
        Print.Section("4-1. lock — Monitor 기반 상호 배제");
        {
            int counter = 0;
            var lockObj = new object();
            Parallel.For(0, 10_000, _ => { lock (lockObj) counter++; });
            Console.WriteLine($"    결과: {counter}  (기대: 10000)");
            Console.WriteLine("    내부: Monitor.Enter / Monitor.Exit");
            Console.WriteLine("    경합 시 스레드를 OS가 재우므로(블록) 컨텍스트 스위치 발생");
        }

        // ── 4-2. Interlocked
        Print.Section("4-2. Interlocked — CPU 원자 명령어 (lock-free)");
        {
            long counter = 0;
            Parallel.For(0, 10_000, _ => Interlocked.Increment(ref counter));
            Console.WriteLine($"    Increment: {counter}");

            // CompareExchange: 원자적 CAS (Compare-And-Swap)
            long expected = counter;
            long original = Interlocked.CompareExchange(ref counter, 0, expected);
            Console.WriteLine($"    CompareExchange: 이전={original}, 현재={counter}");
            Console.WriteLine("    LOCK XADD/CMPXCHG — 커널 진입 없음, 가장 가볍다");
        }

        // ── 4-3. SemaphoreSlim — N개 동시 접근 제한 (쓰로틀링)
        Print.Section("4-3. SemaphoreSlim — 동시성 제한 / 쓰로틀링");
        {
            const int Limit = 3;
            var sem = new SemaphoreSlim(Limit, Limit);
            int concurrent = 0, maxConcurrent = 0;
            var lockObj = new object();

            await Task.WhenAll(Enumerable.Range(0, 12).Select(async i =>
            {
                await sem.WaitAsync();
                try
                {
                    int c = Interlocked.Increment(ref concurrent);
                    lock (lockObj) maxConcurrent = Math.Max(maxConcurrent, c);
                    await Task.Delay(20);
                }
                finally
                {
                    Interlocked.Decrement(ref concurrent);
                    sem.Release();
                }
            }));
            Console.WriteLine($"    최대 동시 실행: {maxConcurrent}  (제한: {Limit})");
            Console.WriteLine("    WaitAsync() → async 친화적. I/O 쓰로틀링(동시 HTTP 요청 수 제한)에 유용");
        }

        // ── 4-4. ReaderWriterLockSlim
        Print.Section("4-4. ReaderWriterLockSlim — 읽기는 동시, 쓰기만 배타적");
        {
            var rwLock = new ReaderWriterLockSlim();
            int value = 0, reads = 0;

            Parallel.For(0, 1_000, i =>
            {
                if (i % 10 == 0) // 10% 쓰기
                {
                    rwLock.EnterWriteLock();
                    try { value++; }
                    finally { rwLock.ExitWriteLock(); }
                }
                else // 90% 읽기 — 여러 스레드 동시 진입 가능
                {
                    rwLock.EnterReadLock();
                    try { _ = value; Interlocked.Increment(ref reads); }
                    finally { rwLock.ExitReadLock(); }
                }
            });
            rwLock.Dispose();
            Console.WriteLine($"    쓰기: {value}회, 읽기: {reads}회");
            Console.WriteLine("    lock: 읽기도 직렬화됨  ←→  RWLockSlim: 읽기는 동시 허용");
            Console.WriteLine("    읽기가 압도적으로 많을 때 throughput 향상");
        }

        // ── 4-5. ManualResetEventSlim — 게이트 패턴
        Print.Section("4-5. ManualResetEventSlim — 일제 출발 게이트");
        {
            var gate = new ManualResetEventSlim(false);
            int readyCount = 0;

            var workers = Enumerable.Range(0, 4).Select(i => Task.Run(() =>
            {
                Console.WriteLine($"    워커 {i}: 준비 완료, 게이트 대기...");
                Interlocked.Increment(ref readyCount);
                gate.Wait(); // Set() 호출 전까지 블록
                Console.WriteLine($"    워커 {i}: 출발!");
            })).ToArray();

            // 모든 워커가 준비될 때까지 대기
            while (Volatile.Read(ref readyCount) < 4) Thread.SpinWait(50);
            Console.WriteLine("    --- 게이트 열림 (Set) ---");
            gate.Set(); // 모든 대기자를 한꺼번에 통과

            await Task.WhenAll(workers);
            // Reset() 호출 시 다시 닫힘 (AutoResetEvent는 하나만 통과 후 자동 닫힘)
        }

        // ── 4-6. CountdownEvent — N개 완료 수집
        Print.Section("4-6. CountdownEvent — N개 신호 수집 후 계속");
        {
            using var countdown = new CountdownEvent(initialCount: 5);

            for (int i = 0; i < 5; i++)
            {
                int idx = i;
                ThreadPool.QueueUserWorkItem(_ =>
                {
                    Thread.Sleep(10 + idx * 10);
                    Console.WriteLine($"    작업 {idx} 완료 (남은 카운트: {countdown.CurrentCount - 1})");
                    countdown.Signal(); // 카운트 1 감소
                });
            }

            countdown.Wait(); // 카운트 0이 될 때까지 블록
            Console.WriteLine("    모든 작업 수집 완료");
        }

        // ── 4-7. Barrier — 다단계 병렬 알고리즘
        Print.Section("4-7. Barrier — 다단계 동기화 (Phase)");
        {
            const int Participants = 3;
            using var barrier = new Barrier(
                participantCount: Participants,
                postPhaseAction: b => Console.WriteLine($"    === Phase {b.CurrentPhaseNumber} 완료 ===")
            );

            await Task.WhenAll(Enumerable.Range(0, Participants).Select(i => Task.Run(() =>
            {
                for (int phase = 0; phase < 2; phase++)
                {
                    Console.WriteLine($"    참가자 {i}: Phase {phase} 작업 중...");
                    Thread.Sleep(20 * (i + 1));
                    barrier.SignalAndWait(); // 모두 도달할 때까지 대기 후 다음 단계
                }
            })));
        }
    }
}
