namespace FSharpFunctional.Tests

open Xunit
open FSharpFunctional

type BasicTests() =
    [<Fact>]
    member _.``pipeline sums squared even numbers`` () =
        Assert.Equal(20, Basics.pipelineExample [ 1; 2; 3; 4 ])

    [<Fact>]
    member _.``factorial is tail recursive result`` () =
        Assert.Equal(120, Basics.factorial 5)

type DomainTests() =
    [<Fact>]
    member _.``email validates at sign`` () =
        match Domain.EmailAddress.TryCreate "a@example.com" with
        | Ok email -> Assert.Equal("a@example.com", email.Value)
        | Error error -> Assert.Fail(error)

    [<Fact>]
    member _.``payment rejects invalid amount`` () =
        match Domain.createPayment 0m with
        | Error(Domain.InvalidAmount amount) -> Assert.Equal(0m, amount)
        | other -> Assert.Fail($"Expected InvalidAmount, got {other}")

type PipelineTests() =
    [<Fact>]
    member _.``parse and invert handles valid and zero values`` () =
        Assert.Equal(Some 20, Pipelines.parseAndInvert "5")
        Assert.Equal(None, Pipelines.parseAndInvert "0")
        Assert.Equal(None, Pipelines.parseAndInvert "abc")

    [<Fact>]
    member _.``result computation expression returns average`` () =
        Assert.Equal(Ok 15, Pipelines.averageOfPositiveInts "10" "20")
        Assert.True(Result.isError (Pipelines.averageOfPositiveInts "-1" "20"))

type SequenceTests() =
    [<Fact>]
    member _.``fibonacci starts with expected values`` () =
        let actual = Sequences.fibonacci |> Seq.take 8 |> Seq.toArray
        Assert.Equal<int array>([| 0; 1; 1; 2; 3; 5; 8; 13 |], actual)

    [<Fact>]
    member _.``memoize computes only once per key`` () =
        let mutable calls = 0

        let memoized =
            Sequences.memoize (fun n ->
                calls <- calls + 1
                n * n)

        Assert.Equal(49, memoized 7)
        Assert.Equal(49, memoized 7)
        Assert.Equal(1, calls)

type AdvancedTests() =
    [<Fact>]
    member _.``counter agent accumulates messages`` () =
        let agent = AsyncWorkflows.createCounterAgent ()
        agent.Post(AsyncWorkflows.Increment 2)
        agent.Post(AsyncWorkflows.Increment 5)

        Assert.Equal(7, agent.PostAndReply AsyncWorkflows.Get)

    [<Fact>]
    member _.``units of measure compute speed`` () =
        let speed = UnitsOfMeasure.speed 100.0<km> 2.0<h>
        Assert.Equal(50.0, float speed)
