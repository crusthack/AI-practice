using BenchmarkDotNet.Running;

BenchmarkRunner.Run<IterBench.B1_LoopPatterns>();
BenchmarkRunner.Run<IterBench.B2_CollectionOps>();
BenchmarkRunner.Run<IterBench.B3_LinqChain>();
