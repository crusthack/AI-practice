namespace _2._HostBuilder.Services;

// Singleton: 앱 전체에서 하나의 인스턴스를 공유하는 카운터
public class CounterService
{
    private int _count;

    public int Increment() => Interlocked.Increment(ref _count);
    public int Decrement() => Interlocked.Decrement(ref _count);
    public int Current   => Volatile.Read(ref _count);
}
