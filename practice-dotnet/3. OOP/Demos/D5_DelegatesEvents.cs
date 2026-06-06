namespace OOP.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 델리게이트 & 이벤트 (Delegates & Events)
//   delegate 선언 · 멀티캐스트 · Action/Func/Predicate
//   event · EventHandler<T> · Observer 패턴
// ────────────────────────────────────────────────────────────────────────────
static class D5_DelegatesEvents
{
    public static void Run()
    {
        Print.Header("5. 델리게이트 & 이벤트");

        ShowDelegateBasics();
        ShowMulticast();
        ShowBuiltinDelegates();
        ShowEvents();
        ShowObserverPattern();
    }

    // ── 5-1. 델리게이트 기초 ────────────────────────────────────────────────
    static void ShowDelegateBasics()
    {
        Print.Section("5-1. 델리게이트 기초 — 선언·인스턴스화·호출");

        // 정적 메서드 참조
        MathOp add = Add;
        Print.Line($"정적 메서드: add(3,4) = {add(3, 4)}");

        // 인스턴스 메서드 참조
        var calc = new Calculator();
        MathOp mul = calc.Multiply;
        Print.Line($"인스턴스 메서드: mul(3,4) = {mul(3, 4)}");

        // 람다
        MathOp sub = (a, b) => a - b;
        Print.Line($"람다: sub(10,4) = {sub(10, 4)}");

        // 익명 메서드 (구식; 람다로 대체 가능)
        MathOp div = delegate (int a, int b) { return a / b; };
        Print.Line($"익명 메서드: div(12,4) = {div(12, 4)}");

        // null 조건 호출 (?.) — int 반환이므로 int?로 받아야 ?.ToString() 가능
        MathOp? maybeNull = null;
        int? nullResult = maybeNull?.Invoke(1, 2);
        Print.Line($"null 델리게이트 호출: {nullResult?.ToString() ?? "null"}");
    }

    static int Add(int a, int b) => a + b;

    class Calculator
    {
        public int Multiply(int a, int b) => a * b;
    }

    // ── 5-2. 멀티캐스트 델리게이트 ─────────────────────────────────────────
    static void ShowMulticast()
    {
        Print.Section("5-2. 멀티캐스트 델리게이트 — +=  -=  GetInvocationList");

        // Logger? 로 선언: -=가 null을 반환할 수 있음 (마지막 핸들러 제거 시)
        Logger? logger = LogToConsole;
        logger += LogToFile;
        logger += LogToMemory;

        Print.Line($"등록된 핸들러 수: {logger!.GetInvocationList().Length}");
        Console.Write("    호출: "); logger("hello");

        // 특정 핸들러 제거 (3→2로, null 아님)
        logger -= LogToFile;
        Print.Line($"LogToFile 제거 후: {logger!.GetInvocationList().Length}개");
        Console.Write("    호출: "); logger("world");
    }

    delegate void Logger(string msg);

    static void LogToConsole(string msg) => Console.Write($"[Console:{msg}] ");
    static void LogToFile(string msg)    => Console.Write($"[File:{msg}] ");
    static void LogToMemory(string msg)  => Console.Write($"[Memory:{msg}] ");

    // ── 5-3. 내장 델리게이트 ────────────────────────────────────────────────
    static void ShowBuiltinDelegates()
    {
        Print.Section("5-3. Action<T> / Func<T,TResult> / Predicate<T>");

        // Action: 반환값 없음
        Action<string> print = msg => Console.WriteLine($"    Action: {msg}");
        print("안녕하세요");

        // Action<T1,T2,...> 최대 16개 타입 파라미터
        Action<int, int, string> log = (a, b, op) =>
            Console.WriteLine($"    Action<3>: {a} {op} {b} = {(op == "+" ? a + b : a * b)}");
        log(3, 4, "+"); log(3, 4, "*");

        // Func: 마지막 타입 파라미터가 반환 형식
        Func<double, double> sqrt = Math.Sqrt;
        Func<int, int, int>  pow  = (b, e) => (int)Math.Pow(b, e);
        Print.Line($"Func<double,double> sqrt(9): {sqrt(9)}");
        Print.Line($"Func<int,int,int>   pow(2,8): {pow(2, 8)}");

        // Predicate<T>: bool 반환 (= Func<T, bool>)
        Predicate<int>    isEven  = n => n % 2 == 0;
        Predicate<string> isLong  = s => s.Length > 5;
        Print.Line($"Predicate isEven(4): {isEven(4)}");
        Print.Line($"Predicate isLong(\"hi\"): {isLong("hi")}");

        // Func 합성
        Func<int, int>    double_   = x => x * 2;
        Func<int, int>    addOne    = x => x + 1;
        Func<int, string> toStr     = x => $"결과={x}";
        Func<int, string> pipeline  = x => toStr(addOne(double_(x)));
        Print.Line($"함수 합성 pipeline(5): {pipeline(5)}");

        // 클로저: 람다가 외부 변수를 캡처
        int multiplier = 3;
        Func<int, int> times = x => x * multiplier;
        multiplier = 10;   // 캡처된 변수가 바뀌면 람다도 영향 받음
        Print.Line($"클로저 캡처(multiplier=10): times(5)={times(5)}");
    }

    // ── 5-4. 이벤트 ─────────────────────────────────────────────────────────
    static void ShowEvents()
    {
        Print.Section("5-4. event — EventHandler<T> 패턴");

        var sensor = new TemperatureSensor("센서-1");

        // += 로 구독
        sensor.TemperatureChanged += OnTemperatureChanged;
        sensor.ThresholdExceeded  += OnThresholdExceeded;

        sensor.ReadTemperature(22.5);
        sensor.ReadTemperature(36.8);
        sensor.ReadTemperature(41.2);   // 임계값(40) 초과 → ThresholdExceeded 발생

        // -= 로 구독 해제
        sensor.TemperatureChanged -= OnTemperatureChanged;
        Print.Line("구독 해제 후 ReadTemperature:");
        sensor.ReadTemperature(30.0);   // 이벤트 발생 안 됨
    }

    static void OnTemperatureChanged(object? sender, TemperatureEventArgs e)
        => Console.WriteLine($"    [이벤트] {e.SensorId}: {e.Previous:F1}→{e.Current:F1}°C");

    static void OnThresholdExceeded(object? sender, TemperatureEventArgs e)
        => Console.WriteLine($"    [경보!]  {e.SensorId}: {e.Current:F1}°C (임계={e.Threshold:F1})");

    // ── 5-5. Observer 패턴 ──────────────────────────────────────────────────
    static void ShowObserverPattern()
    {
        Print.Section("5-5. Observer 패턴 — event로 구현");

        var store  = new StockStore();
        var client1 = new StockClient("Alice");
        var client2 = new StockClient("Bob");

        store.PriceChanged += client1.OnPriceChanged;
        store.PriceChanged += client2.OnPriceChanged;

        store.UpdatePrice("삼성전자", 70_000);
        store.UpdatePrice("카카오",   55_000);

        store.PriceChanged -= client2.OnPriceChanged;
        Print.Line("Bob 구독 해제 후:");
        store.UpdatePrice("삼성전자", 72_000);
    }
}

// ── 델리게이트 선언 ───────────────────────────────────────────────────────

delegate int MathOp(int a, int b);

// ── 온도 센서 + 이벤트 ────────────────────────────────────────────────────

public class TemperatureEventArgs(string sensorId, double prev, double curr, double threshold) : EventArgs
{
    public string SensorId  { get; } = sensorId;
    public double Previous  { get; } = prev;
    public double Current   { get; } = curr;
    public double Threshold { get; } = threshold;
}

public class TemperatureSensor(string id)
{
    private double _last;
    private const double Threshold = 40.0;

    // EventHandler<T>: (object? sender, T e) => void 형태의 표준 델리게이트
    public event EventHandler<TemperatureEventArgs>? TemperatureChanged;
    public event EventHandler<TemperatureEventArgs>? ThresholdExceeded;

    public void ReadTemperature(double value)
    {
        var args = new TemperatureEventArgs(id, _last, value, Threshold);
        _last = value;

        // 보호 호출: 구독자가 없으면 null → ?.Invoke()로 안전하게
        TemperatureChanged?.Invoke(this, args);

        if (value > Threshold)
            ThresholdExceeded?.Invoke(this, args);
    }
}

// ── Observer 패턴 ─────────────────────────────────────────────────────────

public class PriceEventArgs(string ticker, decimal price) : EventArgs
{
    public string  Ticker { get; } = ticker;
    public decimal Price  { get; } = price;
}

public class StockStore
{
    public event EventHandler<PriceEventArgs>? PriceChanged;

    public void UpdatePrice(string ticker, decimal price)
        => PriceChanged?.Invoke(this, new PriceEventArgs(ticker, price));
}

public class StockClient(string name)
{
    public void OnPriceChanged(object? sender, PriceEventArgs e)
        => Console.WriteLine($"    [{name}] {e.Ticker} → {e.Price:C}");
}
