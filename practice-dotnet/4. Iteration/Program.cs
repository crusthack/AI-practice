using Iteration.Demos;

Console.OutputEncoding = System.Text.Encoding.UTF8;
Console.WriteLine("=== C# 반복·조건·컬렉션·이터레이터·LINQ·인터페이스 총정리 ===");
Console.WriteLine("벤치마크: cd \"6. Iteration/Benchmarks\" && dotnet run -c Release\n");

D1_Loops.Run();
D2_Conditionals.Run();
D3_Collections.Run();
D4_Iterators.Run();
await D4_Iterators.RunAsync();
D5_LINQ.Run();
D6_Interfaces.Run();
D7_AdvancedIteration.Run();

Console.WriteLine("\n모든 데모 완료.");
