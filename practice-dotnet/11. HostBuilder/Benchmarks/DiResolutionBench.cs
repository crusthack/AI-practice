using _2._HostBuilder.Options;
using _2._HostBuilder.Services;
using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Options;

// ─────────────────────────────────────────────────────
// DI 서비스 생명주기별 해결(Resolution) 비용 비교
//
// Singleton  : 최초 1회 생성 후 캐시 — Dictionary 룩업만.
// Transient  : 매 요청마다 new 인스턴스 생성.
// Scoped     : CreateScope() 비용 + 스코프 내 캐시 조회.
// ─────────────────────────────────────────────────────
[MemoryDiagnoser]
[ShortRunJob]
public class DiResolutionBench
{
    private ServiceProvider _singletonProvider = null!;
    private ServiceProvider _transientProvider = null!;
    private ServiceProvider _scopedProvider    = null!;

    private static IOptions<AppSettings> DefaultOptions =>
        Options.Create(new AppSettings());

    [GlobalSetup]
    public void Setup()
    {
        _singletonProvider = new ServiceCollection()
            .AddSingleton(DefaultOptions)
            .AddSingleton<IGreetingService, GreetingService>()
            .BuildServiceProvider();

        _transientProvider = new ServiceCollection()
            .AddSingleton(DefaultOptions)
            .AddTransient<IGreetingService, GreetingService>()
            .BuildServiceProvider();

        _scopedProvider = new ServiceCollection()
            .AddSingleton(DefaultOptions)
            .AddScoped<IGreetingService, GreetingService>()
            .BuildServiceProvider();
    }

    [GlobalCleanup]
    public void Cleanup()
    {
        _singletonProvider.Dispose();
        _transientProvider.Dispose();
        _scopedProvider.Dispose();
    }

    [Benchmark(Baseline = true)]
    public IGreetingService ResolveSingleton() =>
        _singletonProvider.GetRequiredService<IGreetingService>();

    [Benchmark]
    public IGreetingService ResolveTransient() =>
        _transientProvider.GetRequiredService<IGreetingService>();

    [Benchmark]
    public IGreetingService ResolveScoped()
    {
        using var scope = _scopedProvider.CreateScope();
        return scope.ServiceProvider.GetRequiredService<IGreetingService>();
    }
}
