using Demos;

Console.OutputEncoding = System.Text.Encoding.UTF8;

Console.WriteLine("=== C# 멀티스레딩 & Task 시스템 데모 ===");
Console.WriteLine("벤치마크: cd \"4. MultiThreading/Benchmarks\" && dotnet run -c Release\n");

await D1_Thread.RunAsync();
await D2_Task.RunAsync();
await D3_Parallel.RunAsync();
await D4_Sync.RunAsync();
await D5_Collections.RunAsync();
await D6_AdvancedChannels.RunAsync();
await D7_AsyncStreams.RunAsync();
await D8_MemoryAndContext.RunAsync();

Console.WriteLine("\n\n모든 데모 완료.");
