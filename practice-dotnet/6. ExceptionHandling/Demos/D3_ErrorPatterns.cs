namespace ExceptionHandling.Demos;

// ── Result<T, TError> ─────────────────────────────────────────────────────────

public readonly struct Result<T, TError>
{
    private readonly T?      _value;
    private readonly TError? _error;

    public bool IsOk    { get; }
    public T    Value   => IsOk ? _value! : throw new InvalidOperationException("Result is error");
    public TError Error => !IsOk ? _error! : throw new InvalidOperationException("Result is ok");

    private Result(T value)       { _value = value; IsOk = true; }
    private Result(TError error)  { _error = error; IsOk = false; }

    public static Result<T, TError> Ok(T value)      => new(value);
    public static Result<T, TError> Fail(TError err) => new(err);

    public Result<TOut, TError> Map<TOut>(Func<T, TOut> f) =>
        IsOk ? Result<TOut, TError>.Ok(f(_value!)) : Result<TOut, TError>.Fail(_error!);

    public Result<TOut, TError> Bind<TOut>(Func<T, Result<TOut, TError>> f) =>
        IsOk ? f(_value!) : Result<TOut, TError>.Fail(_error!);

    public TOut Match<TOut>(Func<T, TOut> ok, Func<TError, TOut> fail) =>
        IsOk ? ok(_value!) : fail(_error!);

    public override string ToString() =>
        IsOk ? $"Ok({_value})" : $"Fail({_error})";
}

// ── Option<T> ────────────────────────────────────────────────────────────────

public readonly struct Option<T>
{
    private readonly T? _value;
    public bool HasValue { get; }
    public T Value => HasValue ? _value! : throw new InvalidOperationException("Option is None");

    private Option(T value) { _value = value; HasValue = true; }

    public static Option<T> Some(T value) => new(value);
    public static Option<T> None          => default;

    public Option<TOut> Map<TOut>(Func<T, TOut> f) =>
        HasValue ? Option<TOut>.Some(f(_value!)) : Option<TOut>.None;

    public T GetValueOrDefault(T fallback) => HasValue ? _value! : fallback;
    public override string ToString() => HasValue ? $"Some({_value})" : "None";
}

// ── 데모 ──────────────────────────────────────────────────────────────────────

static class D3_ErrorPatterns
{
    public static void Run()
    {
        Print.Header("3. 에러 패턴 — Result · Option · TryXxx");
        ShowResultType();
        ShowResultChaining();
        ShowOptionType();
        ShowTryPattern();
        ShowValidationAccumulation();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowResultType()
    {
        Print.Section("3-1. Result<T, TError> — 예외 없는 오류 반환");

        var ok   = ParseAge("25");
        var fail = ParseAge("abc");

        Print.Line($"Ok   결과: {ok}   IsOk={ok.IsOk}");
        Print.Line($"Fail 결과: {fail}  IsOk={fail.IsOk}");

        // Match: 두 경우를 모두 처리
        string msg1 = ok.Match(
            ok:   age  => $"나이: {age}세",
            fail: err  => $"오류: {err}");
        string msg2 = fail.Match(
            ok:   age  => $"나이: {age}세",
            fail: err  => $"오류: {err}");

        Print.Line($"  Match ok:   {msg1}");
        Print.Line($"  Match fail: {msg2}");

        static Result<int, string> ParseAge(string s) =>
            int.TryParse(s, out int age) && age is >= 0 and <= 150
                ? Result<int, string>.Ok(age)
                : Result<int, string>.Fail($"유효하지 않은 나이: '{s}'");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowResultChaining()
    {
        Print.Section("3-2. Map · Bind — 파이프라인 연결");

        // Map: 성공 값 변환 (오류는 그대로 전파)
        var mapResult = ParsePositiveInt("10")
            .Map(n => n * 2)
            .Map(n => $"결과: {n}");
        Print.Line($"  Map 체인:    {mapResult}");

        // Bind: 또 다른 Result 반환 함수로 연결
        var bindOk   = ParsePositiveInt("5").Bind(Sqrt);
        var bindFail = ParsePositiveInt("-3").Bind(Sqrt);
        Print.Line($"  Bind 성공:   {bindOk}");
        Print.Line($"  Bind 실패:   {bindFail}");

        static Result<int, string> ParsePositiveInt(string s) =>
            int.TryParse(s, out int n)
                ? n > 0 ? Result<int, string>.Ok(n)
                        : Result<int, string>.Fail($"{n} <= 0")
                : Result<int, string>.Fail($"파싱 실패: {s}");

        static Result<double, string> Sqrt(int n) =>
            n >= 0 ? Result<double, string>.Ok(Math.Sqrt(n))
                   : Result<double, string>.Fail("음수 제곱근 불가");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowOptionType()
    {
        Print.Section("3-3. Option<T> — null 없는 선택적 값");

        var dict = new Dictionary<string, int> { ["a"] = 1, ["b"] = 2 };

        Option<int> FindValue(string key) =>
            dict.TryGetValue(key, out int v) ? Option<int>.Some(v) : Option<int>.None;

        var found   = FindValue("a");
        var missing = FindValue("z");

        Print.Line($"  found:   {found}  HasValue={found.HasValue}");
        Print.Line($"  missing: {missing}  HasValue={missing.HasValue}");

        // Map: 값이 없으면 변환 건너뜀
        var doubled = found.Map(x => x * 2);
        var none    = missing.Map(x => x * 2);
        Print.Line($"  found.Map(*2):   {doubled}");
        Print.Line($"  missing.Map(*2): {none}");

        // GetValueOrDefault
        Print.Line($"  GetValueOrDefault: {missing.GetValueOrDefault(-1)}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowTryPattern()
    {
        Print.Section("3-4. TryXxx 패턴 — BCL 관례");

        // int.TryParse
        if (int.TryParse("42", out int n))
            Print.Line($"  int.TryParse: {n}");

        // Dictionary.TryGetValue
        var cache = new Dictionary<string, string> { ["key"] = "value" };
        if (cache.TryGetValue("key", out string? val))
            Print.Line($"  TryGetValue: {val}");

        // 커스텀 TryXxx — 예외 없이 bool + out 반환
        if (TryDivide(10, 3, out double result))
            Print.Line($"  TryDivide(10,3): {result:F2}");
        if (!TryDivide(10, 0, out _))
            Print.Line($"  TryDivide(10,0): 실패 (0으로 나누기)");

        static bool TryDivide(double a, double b, out double result)
        {
            if (b == 0) { result = 0; return false; }
            result = a / b;
            return true;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowValidationAccumulation()
    {
        Print.Section("3-5. 유효성 오류 누적 — 첫 번째 오류에서 멈추지 않기");

        var errors = ValidateUser(name: "", email: "invalid", age: -1);
        if (errors.Count > 0)
        {
            Print.Line($"  유효성 오류 {errors.Count}개:");
            foreach (var e in errors)
                Print.Line($"    • {e}");
        }

        errors = ValidateUser("홍길동", "hong@example.com", 30);
        Print.Line($"  유효한 입력 → 오류 수: {errors.Count}");

        static List<string> ValidateUser(string name, string email, int age)
        {
            var errs = new List<string>();
            if (string.IsNullOrWhiteSpace(name))  errs.Add("이름 필수");
            if (!email.Contains('@'))             errs.Add("이메일 형식 오류");
            if (age < 0 || age > 150)             errs.Add("나이 범위: 0~150");
            return errs;
        }
    }
}
