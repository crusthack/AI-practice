using BenchmarkDotNet.Running;

BenchmarkRunner.Run<LinqBench>();
BenchmarkRunner.Run<StringOpsBench>();
