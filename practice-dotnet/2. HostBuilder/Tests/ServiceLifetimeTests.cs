using _2._HostBuilder.Options;
using _2._HostBuilder.Services;
using Microsoft.Extensions.DependencyInjection;
using OptionsFactory = Microsoft.Extensions.Options.Options;

namespace _2._HostBuilder.Tests;

public class ServiceLifetimeTests
{
    [Fact]
    public void Singleton_ReturnsSameInstance()
    {
        var sc = new ServiceCollection();
        sc.AddSingleton<CounterService>();
        using var provider = sc.BuildServiceProvider();

        var a = provider.GetRequiredService<CounterService>();
        var b = provider.GetRequiredService<CounterService>();

        Assert.Same(a, b);
    }

    [Fact]
    public void Transient_ReturnsDifferentInstances()
    {
        var sc = new ServiceCollection();
        sc.AddSingleton(OptionsFactory.Create(new AppSettings()));
        sc.AddTransient<IGreetingService, GreetingService>();
        using var provider = sc.BuildServiceProvider();

        var a = provider.GetRequiredService<IGreetingService>();
        var b = provider.GetRequiredService<IGreetingService>();

        Assert.NotSame(a, b);
    }

    [Fact]
    public void Scoped_SameWithinScope_DifferentAcrossScopes()
    {
        var sc = new ServiceCollection();
        sc.AddSingleton(OptionsFactory.Create(new AppSettings()));
        sc.AddScoped<IGreetingService, GreetingService>();
        using var provider = sc.BuildServiceProvider();

        IGreetingService s1;
        using (var scope = provider.CreateScope())
        {
            s1 = scope.ServiceProvider.GetRequiredService<IGreetingService>();
            var s2 = scope.ServiceProvider.GetRequiredService<IGreetingService>();
            Assert.Same(s1, s2); // 같은 스코프 — 동일 인스턴스
        }

        using (var scope = provider.CreateScope())
        {
            var s3 = scope.ServiceProvider.GetRequiredService<IGreetingService>();
            Assert.NotSame(s1, s3); // 다른 스코프 — 새 인스턴스
        }
    }

    [Fact]
    public void CounterService_Singleton_SharedAcrossResolutions()
    {
        var sc = new ServiceCollection();
        sc.AddSingleton<CounterService>();
        using var provider = sc.BuildServiceProvider();

        var counter = provider.GetRequiredService<CounterService>();
        counter.Increment();
        counter.Increment();

        var sameCounter = provider.GetRequiredService<CounterService>();
        Assert.Equal(2, sameCounter.Current);
    }
}
