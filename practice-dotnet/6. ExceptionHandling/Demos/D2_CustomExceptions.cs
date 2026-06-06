namespace ExceptionHandling.Demos;

// ── 도메인 예외 계층 ───────────────────────────────────────────────────────────

public class AppException(string message, Exception? inner = null)
    : Exception(message, inner)
{
    public string ErrorCode { get; init; } = "APP_ERROR";
}

public class NotFoundException(string resource, object key)
    : AppException($"'{resource}' with key '{key}' not found.")
{
    public string Resource { get; } = resource;
    public object Key      { get; } = key;
    public new string ErrorCode => "NOT_FOUND";
}

public class ValidationException : AppException
{
    public IReadOnlyList<string> Errors { get; }

    public ValidationException(IEnumerable<string> errors)
        : base("유효성 검사 실패: " + string.Join("; ", errors))
    {
        Errors = errors.ToList().AsReadOnly();
        ErrorCode = "VALIDATION_ERROR";
    }
}

public class ConflictException(string message) : AppException(message)
{
    public new string ErrorCode => "CONFLICT";
}

// ── 데모 클래스 ───────────────────────────────────────────────────────────────

static class D2_CustomExceptions
{
    public static void Run()
    {
        Print.Header("2. 커스텀 예외 설계");
        ShowExceptionHierarchy();
        ShowValidationException();
        ShowExceptionWrapping();
        ShowExceptionData();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowExceptionHierarchy()
    {
        Print.Section("2-1. 도메인 예외 계층");

        var cases = new Action[]
        {
            () => throw new NotFoundException("User", 42),
            () => throw new ConflictException("이메일 중복"),
        };

        foreach (var action in cases)
        {
            try { action(); }
            catch (NotFoundException ex)
            {
                Print.Line($"  NOT_FOUND: {ex.Resource}[{ex.Key}] — {ex.Message}");
            }
            catch (AppException ex)
            {
                Print.Line($"  APP_ERROR({ex.ErrorCode}): {ex.Message}");
            }
        }

        // is / as 패턴
        Exception ex2 = new NotFoundException("Order", "ORD-001");
        Print.Line($"  is AppException: {ex2 is AppException}");
        Print.Line($"  is NotFoundException: {ex2 is NotFoundException nfe && nfe.Resource == "Order"}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowValidationException()
    {
        Print.Section("2-2. 다중 오류 집약 — ValidationException");

        try
        {
            ValidateOrder(name: "", quantity: -5, price: 0);
        }
        catch (ValidationException ex)
        {
            Print.Line($"  오류 수: {ex.Errors.Count}");
            foreach (var err in ex.Errors)
                Print.Line($"    • {err}");
        }

        static void ValidateOrder(string name, int quantity, decimal price)
        {
            var errors = new List<string>();
            if (string.IsNullOrWhiteSpace(name))   errors.Add("이름 필수");
            if (quantity <= 0)                      errors.Add("수량 > 0 이어야 함");
            if (price <= 0)                         errors.Add("가격 > 0 이어야 함");
            if (errors.Count > 0) throw new ValidationException(errors);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowExceptionWrapping()
    {
        Print.Section("2-3. 예외 번역 — 저수준 → 도메인 예외");

        try
        {
            LoadUserFromDb(userId: 999);
        }
        catch (AppException ex)
        {
            Print.Line($"  도메인 예외: {ex.Message}");
            Print.Line($"  원인(Inner): {ex.InnerException?.GetType().Name}: {ex.InnerException?.Message}");
        }

        static void LoadUserFromDb(int userId)
        {
            try
            {
                // 저수준 인프라 오류 시뮬레이션 (TimeoutException = DB 타임아웃)
                throw new TimeoutException($"connection timeout (userId={userId})");
            }
            catch (Exception inner)
            {
                // 도메인 예외로 번역: 저수준 세부사항을 숨기고 도메인 언어로
                throw new AppException($"사용자 {userId} 로드 실패", inner)
                {
                    ErrorCode = "DB_ERROR"
                };
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowExceptionData()
    {
        Print.Section("2-4. Exception.Data — 추가 진단 정보");

        var ex = new AppException("주문 처리 실패");
        ex.Data["orderId"]    = "ORD-789";
        ex.Data["userId"]     = 42;
        ex.Data["timestamp"]  = DateTime.UtcNow;
        ex.Data["retryCount"] = 3;

        try { throw ex; }
        catch (AppException caught)
        {
            Print.Line("  Data 사전:");
            foreach (System.Collections.DictionaryEntry kv in caught.Data)
                Print.Line($"    {kv.Key} = {kv.Value}");
        }
    }
}
