using TypeSystem.Demos;

Console.OutputEncoding = System.Text.Encoding.UTF8;
Console.WriteLine("=== C# 타입 시스템 완전 정복 ===");
Console.WriteLine("벤치마크: cd \"5. TypeSystem/Benchmarks\" && dotnet run -c Release\n");

D1_ValueTypes.Run();
D2_ReferenceTypes.Run();
D3_Records.Run();
D4_Syntax.Run();
D5_Boxing.Run();

Console.WriteLine("\n\n모든 데모 완료.");
