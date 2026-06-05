namespace _2._HostBuilder.Options;

public class AppSettings
{
    public const string SectionName = "App";

    public string Name { get; init; } = "HostBuilderDemo";
    public int MaxRetries { get; init; } = 3;
    public TimeSpan WorkerInterval { get; init; } = TimeSpan.FromSeconds(2);
}
