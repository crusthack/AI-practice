using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;

// ───────────────────────────────────────────────────────
// 읽기 편향 접근 패턴 — lock vs ReaderWriterLockSlim
//
// lock              : 읽기·쓰기 모두 직렬화. 독자가 많아도 한 번에 하나씩.
// ReaderWriterLockSlim : 읽기는 동시 허용, 쓰기만 배타적.
//                     독자 수가 많을수록 이득이 커진다.
//
// WriteRatio = 0.1 → 읽기 90%, 쓰기 10%
// ───────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class ReadHeavyBench
{
    [Params(100_000)]
    public int Iterations;

    private const double WriteRatio = 0.10; // 10% 쓰기, 90% 읽기

    private int _value;
    private readonly object _lockObj = new();
    private readonly ReaderWriterLockSlim _rwLock = new(LockRecursionPolicy.NoRecursion);

    [IterationSetup]
    public void Reset() => _value = 0;

    [GlobalCleanup]
    public void Cleanup() => _rwLock.Dispose();

    // 읽기도 lock으로 직렬화 — 독자들이 서로 기다림
    [Benchmark(Baseline = true)]
    public int WithLock()
    {
        Parallel.For(0, Iterations, i =>
        {
            if (i % 10 < (int)(WriteRatio * 10))
            {
                lock (_lockObj) _value++;
            }
            else
            {
                lock (_lockObj) _ = _value;
            }
        });
        return _value;
    }

    // 읽기는 동시 허용 — 독자들이 서로 기다리지 않음
    [Benchmark]
    public int WithReaderWriterLockSlim()
    {
        Parallel.For(0, Iterations, i =>
        {
            if (i % 10 < (int)(WriteRatio * 10))
            {
                _rwLock.EnterWriteLock();
                try { _value++; }
                finally { _rwLock.ExitWriteLock(); }
            }
            else
            {
                _rwLock.EnterReadLock();
                try { _ = _value; }
                finally { _rwLock.ExitReadLock(); }
            }
        });
        return _value;
    }
}
