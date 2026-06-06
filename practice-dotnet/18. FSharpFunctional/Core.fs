namespace FSharpFunctional

open System
open System.Collections.Generic

module Basics =
    let square x = x * x
    let increment x = x + 1
    let double x = x * 2
    let composeExample x = x |> (increment >> double)

    let pipelineExample numbers =
        numbers
        |> List.filter (fun n -> n % 2 = 0)
        |> List.map square
        |> List.sum

    let factorial n =
        if n < 0 then invalidArg (nameof n) "n must be non-negative"

        let rec loop acc remaining =
            match remaining with
            | 0 | 1 -> acc
            | n -> loop (acc * n) (n - 1)

        loop 1 n

    let classifyNumber n =
        match n with
        | 0 -> "zero"
        | n when n < 0 -> "negative"
        | n when n % 2 = 0 -> "positive even"
        | _ -> "positive odd"

module Domain =
    type EmailAddress =
        private
        | EmailAddress of string

        member this.Value =
            let (EmailAddress value) = this
            value

        static member TryCreate(value: string) =
            if String.IsNullOrWhiteSpace value then
                Error "Email cannot be empty."
            elif value.Contains("@") then
                Ok(EmailAddress value)
            else
                Error "Email must contain '@'."

        override this.ToString() = this.Value

    type CustomerId = CustomerId of int

    type Customer =
        { Id: CustomerId
          Name: string
          Email: EmailAddress option }

    type OrderStatus =
        | Draft
        | Paid of paidAt: DateTime
        | Shipped of trackingNumber: string
        | Cancelled of reason: string

    type PaymentError =
        | InvalidAmount of decimal
        | CardExpired
        | InsufficientFunds

    type Payment =
        { Amount: decimal
          Currency: string }

    let createPayment amount =
        if amount <= 0m then
            Error(InvalidAmount amount)
        else
            Ok { Amount = amount; Currency = "USD" }

    let describeStatus status =
        match status with
        | Draft -> "draft"
        | Paid paidAt -> sprintf "paid at %s" (paidAt.ToString("yyyy-MM-dd"))
        | Shipped tracking -> $"shipped: {tracking}"
        | Cancelled reason -> $"cancelled: {reason}"

module Pipelines =
    let parseInt (text: string) =
        match Int32.TryParse text with
        | true, value -> Some value
        | false, _ -> None

    let tryDivide numerator denominator =
        if denominator = 0 then None else Some(numerator / denominator)

    let parseAndInvert text =
        text
        |> parseInt
        |> Option.bind (tryDivide 100)

    let parsePositiveInt text =
        match parseInt text with
        | Some value when value > 0 -> Ok value
        | Some _ -> Error "Value must be positive."
        | None -> Error "Value must be an integer."

    type ResultBuilder() =
        member _.Bind(value, binder) = Result.bind binder value
        member _.Return(value) = Ok value
        member _.ReturnFrom(value) = value

    let result = ResultBuilder()

    let averageOfPositiveInts left right =
        result {
            let! x = parsePositiveInt left
            let! y = parsePositiveInt right
            return (x + y) / 2
        }

module Sequences =
    let naturals = Seq.initInfinite id

    let fibonacci =
        Seq.unfold
            (fun (a, b) -> Some(a, (b, a + b)))
            (0, 1)

    let movingAverage windowSize (values: seq<float>) =
        if windowSize < 1 then invalidArg (nameof windowSize) "windowSize must be positive"

        values
        |> Seq.windowed windowSize
        |> Seq.map (fun window -> window |> Array.averageBy float)

    let memoize (f: 'a -> 'b) =
        let cache = Dictionary<'a, 'b>()

        fun key ->
            match cache.TryGetValue key with
            | true, value -> value
            | false, _ ->
                let value = f key
                cache[key] <- value
                value

module AsyncWorkflows =
    let fetchSimulated id =
        async {
            do! Async.Sleep 5
            return $"item-{id}"
        }

    let fetchAll ids =
        ids
        |> List.map fetchSimulated
        |> Async.Parallel

    type CounterMessage =
        | Increment of int
        | Reset
        | Get of AsyncReplyChannel<int>

    let createCounterAgent () =
        MailboxProcessor.Start(fun inbox ->
            let rec loop count =
                async {
                    let! message = inbox.Receive()

                    match message with
                    | Increment amount -> return! loop (count + amount)
                    | Reset -> return! loop 0
                    | Get reply ->
                        reply.Reply count
                        return! loop count
                }

            loop 0)

[<Measure>]
type km

[<Measure>]
type h

module UnitsOfMeasure =
    let speed (distance: float<km>) (time: float<h>) = distance / time
