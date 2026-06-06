using Containers.Demos;

Console.OutputEncoding = System.Text.Encoding.UTF8;
Console.WriteLine("=== C# 컨테이너 & 제네릭 컬렉션 집대성 ===");
Console.WriteLine("벤치마크: cd \"8. Containers/Benchmarks\" && dotnet run -c Release\n");

D1_SpanMemory.Run();
D2_CustomContainers.Run();
D3_TreeGraph.Run();
D4_BuiltinAdvanced.Run();
D5_Patterns.Run();

Console.WriteLine("\n모든 데모 완료.");
