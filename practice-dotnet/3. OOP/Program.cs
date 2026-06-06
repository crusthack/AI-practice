using OOP.Demos;

Console.OutputEncoding = System.Text.Encoding.UTF8;
Console.WriteLine("=== C# OOP 집대성 ===");
Console.WriteLine("벤치마크: cd \"7. OOP/Benchmarks\" && dotnet run -c Release\n");

D1_Encapsulation.Run();
D2_Inheritance.Run();
D3_Polymorphism.Run();
D4_Generics.Run();
D5_DelegatesEvents.Run();
D6_Composition.Run();
await D7_AdvancedPatterns.RunAsync();

Console.WriteLine("\n모든 데모 완료.");
