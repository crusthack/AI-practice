using _2._HostBuilder.Options;
using _2._HostBuilder.Services;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Options;

HostApplicationBuilder builder = Host.CreateApplicationBuilder(args);

// ── Options 패턴: appsettings.json → AppSettings 자동 바인딩
builder.Services.Configure<AppSettings>(
    builder.Configuration.GetSection(AppSettings.SectionName));

// ── 서비스 등록 — 생명주기 시연
builder.Services.AddSingleton<CounterService>();                    // 전체 수명
builder.Services.AddTransient<IGreetingService, GreetingService>(); // 매번 새 인스턴스

// ── IHostedService 등록 순서 = 시작 순서
builder.Services.AddHostedService<ExampleHostedService>();
builder.Services.AddHostedService<PeriodicWorker>();

using IHost host = builder.Build();

// ── 서비스 직접 사용 예시 (RunAsync 호출 전)
var greeter = host.Services.GetRequiredService<IGreetingService>();
Console.WriteLine(greeter.Greet("World"));

var options = host.Services.GetRequiredService<IOptions<AppSettings>>().Value;
Console.WriteLine($"앱 이름: {options.Name} | 최대 재시도: {options.MaxRetries}");

// ── Ctrl+C 또는 5초 후 자동 종료 (데모용)
using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(5));
Console.CancelKeyPress += (_, e) => { e.Cancel = true; cts.Cancel(); };
Console.WriteLine("(5초 후 자동 종료, 또는 Ctrl+C)\n");

await host.RunAsync(cts.Token);
