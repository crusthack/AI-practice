namespace ExceptionHandling.Demos;

static class D4_ExceptionFilters
{
    public static void Run()
    {
        Print.Header("4. Exception Filter — when 심화");
        ShowConditionalFilter();
        ShowSideEffectFilter();
        ShowNestedConditions();
        ShowFilterWithPattern();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowConditionalFilter()
    {
        Print.Section("4-1. when — 조건부 catch");

        // when false 이면 catch 블록 진입하지 않음 (다음 handler로 전달)
        var codes = new[] { 400, 404, 500, 503 };
        foreach (int code in codes)
        {
            try { ThrowHttpError(code); }
            catch (InvalidOperationException ex) when (ex.Message.StartsWith("4"))
            {
                Print.Line($"  클라이언트 오류 {code}: {ex.Message}");
            }
            catch (InvalidOperationException ex) when (ex.Message.StartsWith("5"))
            {
                Print.Line($"  서버 오류 {code}: {ex.Message}");
            }
            catch (InvalidOperationException ex)
            {
                Print.Line($"  기타 오류 {code}: {ex.Message}");
            }
        }

        static void ThrowHttpError(int code) =>
            throw new InvalidOperationException($"{code} HTTP Error");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowSideEffectFilter()
    {
        Print.Section("4-2. 부수효과 전용 필터 — 잡지 않고 로깅만");

        // when(Log(ex)) 패턴 — Log()가 항상 false를 반환하면
        // catch 블록에 진입하지 않지만 Log()는 실행됨
        try
        {
            try  { throw new InvalidOperationException("DB 연결 실패"); }
            catch (Exception ex) when (Log(ex))  // false 반환 → 진입 안 함
            {
                Print.Line("  이 줄은 실행 안 됨");
            }
        }
        catch (Exception ex)
        {
            Print.Line($"  최종 catch: {ex.Message}");
        }

        // when(Log(ex)) + true: 항상 잡으면서 로깅
        try
        {
            try  { throw new TimeoutException("쿼리 타임아웃"); }
            catch (Exception ex) when (Log(ex) || true)   // true → 항상 진입
            {
                Print.Line($"  로깅+처리: {ex.Message}");
            }
        }
        catch { /* 도달 안 함 */ }

        static bool Log(Exception ex)
        {
            Console.WriteLine($"  [LOG] {DateTime.UtcNow:HH:mm:ss.fff} | {ex.GetType().Name}: {ex.Message}");
            return false;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowNestedConditions()
    {
        Print.Section("4-3. 복합 조건 필터");

        var exceptions = new Exception[]
        {
            new ArgumentException("id", "userId"),
            new ArgumentNullException("email"),
            new InvalidOperationException("transient failure"),
        };

        int attempt = 0;
        foreach (var ex in exceptions)
        {
            attempt++;
            try { throw ex; }
            // 재시도 가능한 ArgumentException만 처리
            catch (ArgumentException e) when (e.ParamName is not null && attempt <= 2)
            {
                Print.Line($"  재시도 가능 ArgEx (param={e.ParamName}): {e.Message}");
            }
            catch (Exception e) when (IsTransient(e))
            {
                Print.Line($"  일시적 오류 — 재시도: {e.Message}");
            }
            catch (Exception e)
            {
                Print.Line($"  처리 불가: {e.GetType().Name} — {e.Message}");
            }
        }

        static bool IsTransient(Exception ex) =>
            ex.Message.Contains("transient") || ex is TimeoutException;
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowFilterWithPattern()
    {
        Print.Section("4-4. when + 패턴 매칭 결합");

        object[] payloads = [404, "404", new ArgumentException("bad param"), null!];

        foreach (var payload in payloads)
        {
            try
            {
                switch (payload)
                {
                    case int code when code >= 400: throw new InvalidOperationException($"HTTP {code}");
                    case string s when int.TryParse(s, out _): throw new FormatException($"int string '{s}'");
                    case Exception ex: throw ex;
                    case null: throw new ArgumentNullException("payload");
                }
            }
            catch (InvalidOperationException ex) when (ex.Message.Contains("HTTP"))
            {
                Print.Line($"  HTTP 오류 패턴: {ex.Message}");
            }
            catch (Exception ex)
            {
                Print.Line($"  기타: {ex.GetType().Name} — {ex.Message}");
            }
        }
    }
}
