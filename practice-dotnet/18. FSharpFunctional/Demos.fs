namespace FSharpFunctional

open System
open FSharpFunctional

module Demos =
    let private header title =
        printfn ""
        printfn "=== %s ===" title

    let private section title =
        printfn ""
        printfn "-- %s" title

    let private printSeq label values =
        values
        |> Seq.map string
        |> String.concat ", "
        |> printfn "    %s: %s" label

    let runBasics () =
        header "1. Beginner - values, functions, pipelines"

        section "1-1. Immutable values and function composition"
        let value = 10
        printfn "    value=%d, composeExample(value)=%d" value (Basics.composeExample value)

        section "1-2. List pipeline"
        let numbers = [ 1..8 ]
        printSeq "source" numbers
        printfn "    sum of squared evens: %d" (Basics.pipelineExample numbers)

        section "1-3. Recursion and pattern matching"
        printfn "    factorial 5 = %d" (Basics.factorial 5)
        [ -3; 0; 4; 7 ]
        |> List.iter (fun n -> printfn "    %d -> %s" n (Basics.classifyNumber n))

    let runDomainModeling () =
        header "2. Intermediate - records, options, results, unions"

        section "2-1. Records and option"
        let email = Domain.EmailAddress.TryCreate "alice@example.com" |> Result.toOption
        let customer: Domain.Customer =
            { Id = Domain.CustomerId 1
              Name = "Alice"
              Email = email }

        printfn "    customer: %s, email exists: %b" customer.Name customer.Email.IsSome

        section "2-2. Discriminated unions"
        [ Domain.Draft
          Domain.Paid(DateTime(2026, 6, 5))
          Domain.Shipped "TRK-001"
          Domain.Cancelled "duplicate" ]
        |> List.iter (Domain.describeStatus >> printfn "    %s")

        section "2-3. Result-based validation"
        [ 100m; 0m; -5m ]
        |> List.iter (fun amount -> printfn "    createPayment(%M) -> %A" amount (Domain.createPayment amount))

    let runPipelines () =
        header "3. Intermediate - option/result pipelines"

        section "3-1. Option.bind"
        [ "5"; "0"; "abc" ]
        |> List.iter (fun text -> printfn "    parseAndInvert \"%s\" -> %A" text (Pipelines.parseAndInvert text))

        section "3-2. Result computation expression"
        printfn "    averageOfPositiveInts 10 20 -> %A" (Pipelines.averageOfPositiveInts "10" "20")
        printfn "    averageOfPositiveInts -1 20 -> %A" (Pipelines.averageOfPositiveInts "-1" "20")

    let runSequences () =
        header "4. Advanced - lazy sequences and memoization"

        section "4-1. Infinite sequences"
        Sequences.naturals |> Seq.skip 5 |> Seq.take 5 |> printSeq "naturals 5..9"
        Sequences.fibonacci |> Seq.take 10 |> printSeq "fibonacci"

        section "4-2. Windowed processing"
        [ 1.0; 2.0; 3.0; 6.0; 10.0 ]
        |> Sequences.movingAverage 3
        |> Seq.map (fun n -> Math.Round(n, 2))
        |> printSeq "moving average"

        section "4-3. Memoization"
        let mutable calls = 0
        let slowSquare n =
            calls <- calls + 1
            n * n

        let memoized = Sequences.memoize slowSquare
        printfn "    first=%d second=%d calls=%d" (memoized 12) (memoized 12) calls

    let runAdvanced () =
        header "5. Advanced - async, agents, units of measure"

        section "5-1. Async workflows"
        [ 1; 2; 3 ]
        |> AsyncWorkflows.fetchAll
        |> Async.RunSynchronously
        |> printSeq "fetched"

        section "5-2. MailboxProcessor actor"
        let agent = AsyncWorkflows.createCounterAgent ()
        agent.Post(AsyncWorkflows.Increment 3)
        agent.Post(AsyncWorkflows.Increment 7)
        let count = agent.PostAndReply AsyncWorkflows.Get
        printfn "    counter=%d" count

        section "5-3. Units of measure"
        let v = UnitsOfMeasure.speed 120.0<km> 2.0<h>
        printfn "    speed = %.1f km/h" (float v)

    let runAll () =
        runBasics ()
        runDomainModeling ()
        runPipelines ()
        runSequences ()
        runAdvanced ()
