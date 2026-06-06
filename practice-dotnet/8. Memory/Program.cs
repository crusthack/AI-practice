using Memory.Demos;

Console.WriteLine("=== C# 메모리 관리 & GC 집대성 ===");
Console.WriteLine("벤치마크: cd \"9. Memory/Benchmarks\" && dotnet run -c Release");
Console.WriteLine();

D1_GCBasics.Run();
D2_LOH.Run();
D3_Finalizer.Run();
D4_Dispose.Run();
D5_UnsafeMemory.Run();

Console.WriteLine("\n모든 데모 완료.");
