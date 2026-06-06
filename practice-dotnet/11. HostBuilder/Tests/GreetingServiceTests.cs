using _2._HostBuilder.Options;
using _2._HostBuilder.Services;
using OptionsFactory = Microsoft.Extensions.Options.Options;

namespace _2._HostBuilder.Tests;

public class GreetingServiceTests
{
    [Fact]
    public void Greet_ReturnsExpectedMessage()
    {
        var options = OptionsFactory.Create(new AppSettings { Name = "TestApp", MaxRetries = 5 });
        var service = new GreetingService(options);

        var result = service.Greet("Alice");

        Assert.Contains("TestApp", result);
        Assert.Contains("Alice",   result);
        Assert.Contains("5",       result);
    }

    [Fact]
    public void Greet_DefaultSettings_UsesDefaults()
    {
        var options = OptionsFactory.Create(new AppSettings());
        var service = new GreetingService(options);

        var result = service.Greet("Bob");

        Assert.Contains("HostBuilderDemo", result);
        Assert.Contains("Bob",             result);
    }
}
