namespace ExceptionHandling.Demos;

static class D1_TryCatchFinally
{
    public static void Run()
    {
        Print.Header("1. try / catch / finally 기본");
        ShowBasicTryCatch();
        ShowMultipleCatch();
        ShowExceptionFilters();
        ShowFinallyBehavior();
        ShowRethrowDifference();
        ShowExceptionProperties();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowBasicTryCatch()
    {
        Print.Section("1-1. 기본 try / catch / finally");

        try
        {
            Print.Line("try 블록 시작");
            int[] arr = [1, 2, 3];
            _ = arr[10];         // IndexOutOfRangeException
            Print.Line("이 줄은 실행 안 됨");
        }
        catch (IndexOutOfRangeException ex)
        {
            Print.Line($"catch: {ex.GetType().Name} — {ex.Message}");
        }
        finally
        {
            Print.Line("finally: 항상 실행 (리소스 해제 여기에)");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowMultipleCatch()
    {
        Print.Section("1-2. 다중 catch — 구체적 타입 먼저");

        foreach (var input in new object[] { null!, "abc", "123", 0 })
        {
            try
            {
                if (input is null)    throw new ArgumentNullException(nameof(input));
                if (input is string s && !int.TryParse(s, out _))
                    throw new FormatException($"'{s}' 는 정수 아님");
                if (input is int i && i == 0)
                    throw new DivideByZeroException();

                Print.Line($"  '{input}' → 정상 처리");
            }
            catch (ArgumentNullException ex) { Print.Line($"  null 오류: {ex.ParamName}"); }
            catch (FormatException ex)       { Print.Line($"  형식 오류: {ex.Message}"); }
            catch (Exception ex)             { Print.Line($"  기타 오류: {ex.GetType().Name}"); }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowExceptionFilters()
    {
        Print.Section("1-3. Exception Filter — when 키워드");

        var exceptions = new Exception[]
        {
            new HttpRequestException("Not Found", null, System.Net.HttpStatusCode.NotFound),
            new HttpRequestException("Server Error", null, System.Net.HttpStatusCode.InternalServerError),
            new InvalidOperationException("상태 오류"),
        };

        foreach (var ex in exceptions)
        {
            try { throw ex; }
            // when 조건으로 특정 HTTP 코드만 처리
            catch (HttpRequestException e) when (e.StatusCode == System.Net.HttpStatusCode.NotFound)
            {
                Print.Line($"  404 처리: {e.Message}");
            }
            catch (HttpRequestException e) when (e.StatusCode >= System.Net.HttpStatusCode.InternalServerError)
            {
                Print.Line($"  5xx 처리: {e.Message}");
            }
            catch (Exception e)
            {
                Print.Line($"  기타: {e.GetType().Name} — {e.Message}");
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowFinallyBehavior()
    {
        Print.Section("1-4. finally 실행 보장 — return / break 포함");

        Print.Line($"  조기 return 결과: {EarlyReturn()}");

        static int EarlyReturn()
        {
            try   { return 42; }
            finally { Console.WriteLine("  finally 실행됨 (return 직전)"); }
        }

        // using 블록 = finally { Dispose() } 와 동일
        var sw = System.Diagnostics.Stopwatch.StartNew();
        try { /* 작업 */ }
        finally { sw.Stop(); Print.Line($"  Stopwatch 정지: {sw.ElapsedMilliseconds}ms"); }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowRethrowDifference()
    {
        Print.Section("1-5. throw vs throw ex — 스택 추적 보존");

        // throw: 원본 스택 추적 유지
        try
        {
            try  { ThrowDeep(); }
            catch (Exception ex)
            {
                // throw ex; ← 스택 추적 초기화 (NG)
                // throw;    ← 원본 스택 추적 유지 (권장)
                Print.Line($"  스택 줄 수 (throw 전): {ex.StackTrace?.Split('\n').Length}");
                throw;
            }
        }
        catch (Exception ex)
        {
            Print.Line($"  스택 줄 수 (throw 후):  {ex.StackTrace?.Split('\n').Length}  ← 동일 유지");
        }

        static void ThrowDeep() => throw new InvalidOperationException("깊은 호출에서 발생");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowExceptionProperties()
    {
        Print.Section("1-6. Exception 프로퍼티 — Message · Data · HResult");

        try
        {
            var ex = new ArgumentException("잘못된 인수", "paramName")
            {
                Source = "ExceptionHandling.Demo"
            };
            ex.Data["userId"]  = 99;
            ex.Data["action"]  = "CreateOrder";
            throw ex;
        }
        catch (ArgumentException ex)
        {
            Print.Line($"  Message:   {ex.Message}");
            Print.Line($"  ParamName: {ex.ParamName}");
            Print.Line($"  Source:    {ex.Source}");
            Print.Line($"  HResult:   0x{ex.HResult:X8}");
            Print.Line($"  Data:      userId={ex.Data["userId"]}, action={ex.Data["action"]}");
        }

        // InnerException 체이닝
        try
        {
            try   { throw new IOException("디스크 읽기 실패"); }
            catch (IOException inner)
            { throw new InvalidOperationException("데이터 로드 실패", inner); }
        }
        catch (InvalidOperationException ex)
        {
            Print.Line($"  Outer:   {ex.Message}");
            Print.Line($"  Inner:   {ex.InnerException?.Message}");
            Print.Line($"  GetBaseException: {ex.GetBaseException().GetType().Name}");
        }
    }
}
