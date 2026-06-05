using BenchmarkDotNet.Running;

// --filter 플래그가 있으면 BenchmarkSwitcher 로 필터링 실행
// 없으면 전체 순차 실행
if (args.Length > 0)
{
    BenchmarkSwitcher.FromAssembly(typeof(Program).Assembly).Run(args);
}
else
{
    BenchmarkRunner.Run<ThreadVsTaskBench>();
    BenchmarkRunner.Run<CpuBoundBench>();
    BenchmarkRunner.Run<IoBoundBench>();
    BenchmarkRunner.Run<SharedStateBench>();
    BenchmarkRunner.Run<ReadHeavyBench>();
    BenchmarkRunner.Run<ProducerConsumerBench>();
    BenchmarkRunner.Run<ParallelAggregateBench>();
    BenchmarkRunner.Run<ActorCounterBench>();
    BenchmarkRunner.Run<FSharpOptParallelBench>();
    BenchmarkRunner.Run<FSharpOptActorBench>();
}
