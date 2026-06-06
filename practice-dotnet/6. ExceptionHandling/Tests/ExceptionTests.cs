using ExceptionHandling.Demos;
using Xunit;

public class TryCatchTests
{
    [Fact] public void ExceptionFilter_When_OnlyMatchingCaught()
    {
        bool caught = false;
        try { throw new InvalidOperationException("400 error"); }
        catch (InvalidOperationException ex) when (ex.Message.StartsWith("4")) { caught = true; }
        Assert.True(caught);
    }

    [Fact] public void Rethrow_PreservesStackTrace()
    {
        string? original = null;
        try
        {
            try { throw new Exception("test"); }
            catch (Exception ex) { original = ex.StackTrace; throw; }
        }
        catch (Exception ex) { Assert.Equal(original, ex.StackTrace); }
    }

    [Fact] public void Finally_RunsOnReturn()
    {
        bool finallyRan = false;
        int val = TryReturn(ref finallyRan);
        Assert.Equal(1, val);
        Assert.True(finallyRan);

        static int TryReturn(ref bool flag)
        {
            try   { return 1; }
            finally { flag = true; }
        }
    }

    [Fact] public void ExceptionData_Dictionary_Works()
    {
        var ex = new Exception("test");
        ex.Data["key"] = "value";
        try { throw ex; }
        catch (Exception caught) { Assert.Equal("value", caught.Data["key"]); }
    }

    [Fact] public void InnerException_Chained()
    {
        var inner = new IOException("IO failure");
        var outer = new InvalidOperationException("Outer", inner);
        Assert.Same(inner, outer.InnerException);
        Assert.Same(inner, outer.GetBaseException());
    }
}

public class CustomExceptionTests
{
    [Fact] public void NotFoundException_MessageContainsKey()
    {
        var ex = new NotFoundException("User", 42);
        Assert.Contains("42", ex.Message);
        Assert.Equal("User", ex.Resource);
        Assert.Equal(42, (int)ex.Key);
    }

    [Fact] public void ValidationException_CollectsAllErrors()
    {
        var errors = new[] { "이름 필수", "이메일 형식 오류" };
        var ex = new ValidationException(errors);
        Assert.Equal(2, ex.Errors.Count);
        Assert.Contains("이름 필수", ex.Errors);
    }

    [Fact] public void AppException_IsBaseOfDomainExceptions()
    {
        Assert.True(new NotFoundException("R", 1) is AppException);
        Assert.True(new ConflictException("dup") is AppException);
        Assert.True(new ValidationException(["e"]) is AppException);
    }
}

public class ResultTypeTests
{
    [Fact] public void Result_Ok_IsOk()
    {
        var r = Result<int, string>.Ok(42);
        Assert.True(r.IsOk);
        Assert.Equal(42, r.Value);
    }

    [Fact] public void Result_Fail_IsNotOk()
    {
        var r = Result<int, string>.Fail("error");
        Assert.False(r.IsOk);
        Assert.Equal("error", r.Error);
    }

    [Fact] public void Result_Map_TransformsValue()
    {
        var r = Result<int, string>.Ok(5).Map(x => x * 2);
        Assert.Equal(10, r.Value);
    }

    [Fact] public void Result_Map_PropagatesError()
    {
        var r = Result<int, string>.Fail("err").Map(x => x * 2);
        Assert.False(r.IsOk);
        Assert.Equal("err", r.Error);
    }

    [Fact] public void Result_Bind_ChainedSuccess()
    {
        var r = Result<int, string>.Ok(4)
            .Bind(n => n > 0
                ? Result<string, string>.Ok($"positive {n}")
                : Result<string, string>.Fail("negative"));
        Assert.True(r.IsOk);
        Assert.Contains("4", r.Value);
    }

    [Fact] public void Option_Some_HasValue()
    {
        var opt = Option<int>.Some(7);
        Assert.True(opt.HasValue);
        Assert.Equal(7, opt.Value);
    }

    [Fact] public void Option_None_GetValueOrDefault()
    {
        var opt = Option<int>.None;
        Assert.Equal(-1, opt.GetValueOrDefault(-1));
    }
}

public class AggregateExceptionTests
{
    [Fact] public void Parallel_AggregateException_Flatten()
    {
        AggregateException? caught = null;
        try
        {
            Parallel.For(0, 4, i =>
            {
                if (i % 2 == 0) throw new InvalidOperationException($"i={i}");
            });
        }
        catch (AggregateException ae) { caught = ae; }

        Assert.NotNull(caught);
        Assert.All(caught!.Flatten().InnerExceptions,
            ex => Assert.IsType<InvalidOperationException>(ex));
    }

    [Fact] public void AggregateException_Handle_ConsumesMatched()
    {
        var ae = new AggregateException(
            new InvalidOperationException("handled"),
            new OutOfMemoryException("unhandled"));

        Assert.Throws<AggregateException>(() =>
            ae.Handle(ex => ex is InvalidOperationException));
    }
}
