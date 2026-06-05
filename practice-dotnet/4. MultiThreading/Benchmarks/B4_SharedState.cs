using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;

// ───────────────────────────────────────────────────────
// 공유 상태 동기화 — 병렬 카운터 증가로 원시 타입 비교
//
// lock (Monitor)  : OS 뮤텍스. 경합 시 스레드 슬립 → 컨텍스트 스위치
// Interlocked     : CPU 원자 명령어(LOCK XADD). 커널 진입 없음
// SpinLock        : CPU 루프로 대기. 임계구역이 극히 짧고 경합이
//                   거의 없을 때만 유리. 경합 심하면 CPU 낭비로 역효과
//
// 예측: Interlocked << lock <<< SpinLock (경합 심한 경우)
// ───────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class SharedStateBench
{
    [Params(100_000)]
    public int Iterations;

    private long _counter;
    private readonly object _lockObj = new();
    private SpinLock _spinLock = new(enableThreadOwnerTracking: false);

    [IterationSetup]
    public void Reset() => _counter = 0;

    // Monitor.Enter/Exit — 경합 시 OS가 스레드를 재움
    [Benchmark(Baseline = true)]
    public long WithLock()
    {
        Parallel.For(0, Iterations, _ =>
        {
            lock (_lockObj) _counter++;
        });
        return _counter;
    }

    // CPU 원자 명령어 하나 — 커널 진입 없음
    [Benchmark]
    public long WithInterlocked()
    {
        Parallel.For(0, Iterations, _ =>
        {
            Interlocked.Increment(ref _counter);
        });
        return _counter;
    }

    // 짧은 임계구역 + 경합 낮을 때 유리
    // 경합이 심하면 모든 스레드가 CPU를 태우며 회전 → 오히려 느림
    [Benchmark]
    public long WithSpinLock()
    {
        Parallel.For(0, Iterations, _ =>
        {
            bool taken = false;
            _spinLock.Enter(ref taken);
            try { _counter++; }
            finally { if (taken) _spinLock.Exit(); }
        });
        return _counter;
    }
}
