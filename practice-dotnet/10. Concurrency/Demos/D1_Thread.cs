namespace Demos;

// ───────────────────────────────────────────────────────
// Thread & ThreadPool
//
// Thread  : OS가 직접 관리하는 네이티브 스레드.
//           스택 기본 1 MB, 생성·파괴 비용이 크다.
//           직접 스레드를 제어해야 할 때만 사용한다.
//
// ThreadPool : .NET 런타임이 관리하는 스레드 풀.
//              스레드를 재사용하므로 생성 비용이 없다.
//              Task.Run, PLINQ, Timer 등의 하부 구현체다.
// ───────────────────────────────────────────────────────
static class D1_Thread
{
    public static async Task RunAsync()
    {
        Print.Header("1. Thread & ThreadPool");

        // ── 1-1. Thread 직접 생성
        Print.Section("1-1. new Thread() — OS 스레드 직접 생성");
        {
            var thread = new Thread(state =>
            {
                Console.WriteLine($"    TID={Environment.CurrentManagedThreadId}  " +
                                  $"IsBackground={Thread.CurrentThread.IsBackground}  " +
                                  $"Name={Thread.CurrentThread.Name}");
            });
            thread.Name = "DemoThread";
            thread.IsBackground = true; // false면 Main 종료 후에도 프로세스 살아있음
            thread.Start();
            thread.Join();              // 완료 대기
        }

        // ── 1-2. 포어그라운드 vs 백그라운드
        Print.Section("1-2. 포어그라운드 vs 백그라운드 스레드");
        Console.WriteLine("    IsBackground=false (기본): Main 종료 후에도 스레드 완료까지 프로세스 유지");
        Console.WriteLine("    IsBackground=true        : Main 종료 시 강제 종료됨");
        Console.WriteLine("    Task.Run으로 만든 스레드  : 항상 백그라운드");

        // ── 1-3. ThreadPool.QueueUserWorkItem
        Print.Section("1-3. ThreadPool.QueueUserWorkItem — 재사용 스레드");
        {
            using var done = new SemaphoreSlim(0, 1);
            ThreadPool.QueueUserWorkItem(_ =>
            {
                Console.WriteLine($"    ThreadPool 스레드 TID={Environment.CurrentManagedThreadId}");
                done.Release();
            });
            await done.WaitAsync(); // 완료 대기
        }

        // ── 1-4. ThreadPool 설정 확인
        Print.Section("1-4. ThreadPool 설정 — 코어 수에 따라 자동 조정됨");
        {
            ThreadPool.GetMinThreads(out int minWorker, out int minIo);
            ThreadPool.GetMaxThreads(out int maxWorker, out int maxIo);
            Console.WriteLine($"    Worker : min={minWorker,4}  max={maxWorker}");
            Console.WriteLine($"    I/O CP : min={minIo,4}  max={maxIo}");
        }

        // ── 1-5. Thread vs Task.Run 선택 기준
        Print.Section("1-5. Thread vs Task.Run — 언제 뭘 써야 하나?");
        Console.WriteLine("    Thread  → 스레드 우선순위·이름·아파트 모델(COM)이 필요할 때");
        Console.WriteLine("    Task.Run → 일반 CPU 바운드 작업, ThreadPool 재사용으로 효율적");
        Console.WriteLine("    async/await → I/O 바운드 작업, 스레드를 블록하지 않음");
    }
}
