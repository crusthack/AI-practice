using System.Globalization;

namespace StringsAndIO.Demos;

static class D2_DateTime
{
    public static void Run()
    {
        Print.Header("2. DateTime · DateTimeOffset · TimeZone · TimeSpan");
        ShowDateTimeBasics();
        ShowDateTimeOffset();
        ShowTimeZone();
        ShowTimeSpan();
        ShowDateOnlyTimeOnly();
        ShowParsing();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowDateTimeBasics()
    {
        Print.Section("2-1. DateTime — 로컬 vs UTC");

        DateTime local = DateTime.Now;
        DateTime utc   = DateTime.UtcNow;

        Print.Line($"Local:     {local:yyyy-MM-dd HH:mm:ss}  Kind={local.Kind}");
        Print.Line($"UTC:       {utc:yyyy-MM-dd HH:mm:ss}    Kind={utc.Kind}");
        Print.Line($"차이:      {local - utc}  (시간대 오프셋)");

        // 날짜 연산
        DateTime deadline = new(2025, 12, 31);
        DateTime today    = DateTime.Today;
        Print.Line($"D-day:     {(deadline - today).Days}일");

        // 구성 요소
        Print.Line($"연월일:    {local.Year}-{local.Month:D2}-{local.Day:D2}");
        Print.Line($"시분초:    {local.Hour:D2}:{local.Minute:D2}:{local.Second:D2}");
        Print.Line($"DayOfWeek: {local.DayOfWeek}  DayOfYear: {local.DayOfYear}");
        Print.Line($"Ticks:     {local.Ticks}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowDateTimeOffset()
    {
        Print.Section("2-2. DateTimeOffset — 시간대 안전");

        // DateTimeOffset은 오프셋을 함께 저장 → 시간대 정보 보존
        var dto = DateTimeOffset.Now;
        var dtoUtc = DateTimeOffset.UtcNow;

        Print.Line($"Now:       {dto:yyyy-MM-dd HH:mm:ss zzz}");
        Print.Line($"Offset:    {dto.Offset}");
        Print.Line($"UTC equiv: {dto.ToUniversalTime():yyyy-MM-dd HH:mm:ss} Z");

        // 특정 오프셋으로 생성
        var kst = new DateTimeOffset(2024, 6, 15, 12, 0, 0,
            TimeSpan.FromHours(9)); // KST +09:00
        Print.Line($"KST:       {kst:yyyy-MM-dd HH:mm:ss zzz}");
        Print.Line($"KST→UTC:   {kst.ToUniversalTime():HH:mm:ss}");

        // 비교는 UTC 기준으로 동일하게
        var utcSame = kst.ToOffset(TimeSpan.Zero);
        Print.Line($"동등 비교: {kst == utcSame}");  // true
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowTimeZone()
    {
        Print.Section("2-3. TimeZoneInfo — 시간대 변환");

        var utcNow = DateTimeOffset.UtcNow;

        var zones = new[] { "Asia/Seoul", "America/New_York", "Europe/London" };
        foreach (var id in zones)
        {
            try
            {
                var tz   = TimeZoneInfo.FindSystemTimeZoneById(id);
                var conv = TimeZoneInfo.ConvertTime(utcNow, tz);
                Print.Line($"  {id,-25}: {conv:HH:mm:ss zzz}  ({tz.DisplayName[..Math.Min(20,tz.DisplayName.Length)]})");
            }
            catch (TimeZoneNotFoundException)
            {
                // Windows vs Linux 시간대 ID 다름
                Print.Line($"  {id}: (이 OS에서 지원 안 됨)");
            }
        }

        // IsDaylightSavingTime
        var seoulTz = TimeZoneInfo.Local;
        Print.Line($"  로컬 DST 적용 중: {seoulTz.IsDaylightSavingTime(utcNow)}");
        Print.Line($"  로컬 UTC 오프셋:  {seoulTz.GetUtcOffset(utcNow)}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowTimeSpan()
    {
        Print.Section("2-4. TimeSpan — 시간 간격");

        var ts1 = TimeSpan.FromHours(2.5);
        var ts2 = new TimeSpan(days: 1, hours: 3, minutes: 20, seconds: 0);

        Print.Line($"ts1:           {ts1}");
        Print.Line($"ts2:           {ts2}");
        Print.Line($"ts1 + ts2:     {ts1 + ts2}");
        Print.Line($"ts2.TotalHours:{ts2.TotalHours:F2}");

        // 성능 측정 패턴
        var sw = System.Diagnostics.Stopwatch.StartNew();
        Thread.Sleep(10);
        sw.Stop();
        Print.Line($"Stopwatch:     {sw.Elapsed.TotalMilliseconds:F1}ms");

        // 타임아웃 패턴
        var timeout = TimeSpan.FromSeconds(30);
        Print.Line($"Timeout:       {timeout.TotalSeconds}초  = {(int)timeout.TotalMilliseconds}ms");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowDateOnlyTimeOnly()
    {
        Print.Section("2-5. DateOnly · TimeOnly (.NET 6+)");

        DateOnly date = new(2024, 6, 15);
        TimeOnly time = new(14, 30, 0);

        Print.Line($"DateOnly: {date}  DayOfWeek: {date.DayOfWeek}");
        Print.Line($"TimeOnly: {time}  IsBetween: {time.IsBetween(new(9,0), new(18,0))}");

        // 날짜 연산
        DateOnly nextWeek = date.AddDays(7);
        Print.Line($"다음 주:  {nextWeek}");

        // DateTime과 변환
        DateTime dt = date.ToDateTime(time);
        Print.Line($"→ DateTime: {dt:yyyy-MM-dd HH:mm:ss}");

        DateOnly fromDt = DateOnly.FromDateTime(DateTime.Now);
        TimeOnly timeFromDt = TimeOnly.FromDateTime(DateTime.Now);
        Print.Line($"현재: {fromDt} {timeFromDt:HH:mm:ss}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowParsing()
    {
        Print.Section("2-6. 파싱 · 형식 변환");

        // Parse / TryParse
        if (DateTime.TryParse("2024-06-15 14:30:00", out var dt1))
            Print.Line($"TryParse: {dt1}");

        // 정확한 형식 지정
        if (DateTime.TryParseExact("15/06/2024", "dd/MM/yyyy",
            CultureInfo.InvariantCulture, DateTimeStyles.None, out var dt2))
            Print.Line($"Exact: {dt2:yyyy-MM-dd}");

        // DateTimeOffset 파싱
        if (DateTimeOffset.TryParse("2024-06-15T14:30:00+09:00", out var dto))
            Print.Line($"DTO:   {dto:yyyy-MM-dd HH:mm:ss zzz}");

        // ISO 8601 형식
        var iso = DateTime.UtcNow.ToString("o"); // ISO 8601 round-trip
        Print.Line($"ISO 8601: {iso}");
        if (DateTime.TryParse(iso, null, DateTimeStyles.RoundtripKind, out var rt))
            Print.Line($"Round-trip Kind: {rt.Kind}");
    }
}
