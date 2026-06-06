using BenchmarkDotNet.Running;

// 모든 벤치마크를 순서대로 실행
// 개별 실행: dotnet run -c Release --filter *StructVsClass*
BenchmarkRunner.Run<StructVsClassBench>();
BenchmarkRunner.Run<BoxingCostBench>();
BenchmarkRunner.Run<DefensiveCopyBench>();
BenchmarkRunner.Run<RecordEqualityBench>();
