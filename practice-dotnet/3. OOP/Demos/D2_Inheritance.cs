namespace OOP.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 상속 (Inheritance)
//   단일 상속 · base 키워드 · virtual/override · new(메서드 숨김)
//   abstract class · sealed · 공변 반환 형식(C# 9) · 생성자 체이닝
// ────────────────────────────────────────────────────────────────────────────
static class D2_Inheritance
{
    public static void Run()
    {
        Print.Header("2. 상속 (Inheritance)");

        ShowBasicInheritance();
        ShowMethodHiding();
        ShowAbstractAndSealed();
        ShowCovariantReturn();
        ShowConstructorChaining();
    }

    // ── 2-1. 기본 상속 ──────────────────────────────────────────────────────
    static void ShowBasicInheritance()
    {
        Print.Section("2-1. 기본 상속 — virtual / override / base");

        Animal[] animals = [new Dog("뭉치"), new Cat("나비"), new Fish("니모")];

        foreach (var a in animals)
        {
            // 다형성: 참조 타입이 Animal이어도 실제 타입의 메서드 호출
            Print.Line($"{a.GetType().Name,-6} → {a.Speak(),-20} | {a.Describe()}");
        }

        // base 키워드: 부모 메서드 명시적 호출
        var dog = new Dog("해피");
        Print.Line($"Dog.Describe (base 포함): {dog.Describe()}");
    }

    // ── 2-2. 메서드 숨김(new) vs 재정의(override) ──────────────────────────
    static void ShowMethodHiding()
    {
        Print.Section("2-2. new(숨김) vs override(재정의) 차이");

        OverrideChild oc = new();
        HidingChild   hc = new();

        // override: 참조 타입이 Base여도 자식 메서드 실행
        Base b1 = oc;
        Print.Line($"override  Base ref  → {b1.Greet()}");   // OverrideChild

        // new(hiding): 참조 타입이 Base면 부모 메서드 실행
        Base b2 = hc;
        Print.Line($"new(hide) Base ref  → {b2.Greet()}");   // Base
        Print.Line($"new(hide) Child ref → {hc.Greet()}");   // HidingChild
    }

    // ── 2-3. 추상 클래스 & sealed ───────────────────────────────────────────
    static void ShowAbstractAndSealed()
    {
        Print.Section("2-3. abstract class & sealed class");

        Shape[] shapes = [new Circle(5), new Rectangle(4, 6)];
        foreach (var s in shapes)
        {
            Print.Line($"{s.GetType().Name,-12} 넓이={s.Area():F2}  둘레={s.Perimeter():F2}  설명={s.Describe()}");
        }

        // sealed: 더 이상 상속 불가
        // class Derived : Circle { }  → CS0509 컴파일 오류
        Print.Line("sealed Circle은 추가 상속 불가 (CS0509)");
    }

    // ── 2-4. 공변 반환 형식 (C# 9+) ────────────────────────────────────────
    static void ShowCovariantReturn()
    {
        Print.Section("2-4. 공변 반환 형식 (Covariant Return, C# 9+)");

        Animal a = new Dog("원본");
        // Clone()은 Animal 반환이지만, Dog.Clone()은 실제로 Dog를 반환
        Animal clone = a.Clone();
        Print.Line($"a.Clone() 실제 타입: {clone.GetType().Name}  (Animal 참조, Dog 인스턴스)");

        // 직접 Dog 참조로 받으면 Dog 메서드에 접근 가능
        Dog dog   = new Dog("독");
        Dog dogC  = dog.Clone();   // override Dog Clone() — Dog 반환
        Print.Line($"dog.Clone() → {dogC.GetType().Name} (캐스팅 불필요)");
    }

    // ── 2-5. 생성자 체이닝 ─────────────────────────────────────────────────
    static void ShowConstructorChaining()
    {
        Print.Section("2-5. 생성자 체이닝 — 부모 생성자 순서");

        // 순서: Vehicle 정적 → Vehicle 인스턴스 → Car 인스턴스
        var log = new List<string>();
        var car = new Car("현대", "소나타", log);
        foreach (var entry in log) Print.Line(entry);
    }
}

// ── Animal 계층 ────────────────────────────────────────────────────────────

public abstract class Animal(string name)
{
    public string Name { get; } = name;

    // virtual: 자식이 재정의 가능 (기본 구현 있음)
    public virtual string Speak()    => "...";
    public virtual string Describe() => $"이름={Name}";

    // 공변 반환: Animal → 파생 클래스가 자신의 타입으로 좁힐 수 있음
    public virtual Animal Clone() => (Animal)MemberwiseClone();
}

public abstract class Mammal(string name) : Animal(name)
{
    public string Breathe() => "폐로 호흡";
}

public sealed class Dog(string name) : Mammal(name)
{
    public override string Speak()    => "멍멍!";
    // base 명시적 호출: 부모 결과에 추가
    public override string Describe() => base.Describe() + $", 종=개";

    // 공변 반환: override Dog (Animal보다 좁은 타입)
    public override Dog Clone() => new Dog(Name);
}

public sealed class Cat(string name) : Mammal(name)
{
    public override string Speak() => "야옹~";
}

public sealed class Fish(string name) : Animal(name)
{
    public override string Speak() => "...뻐끔";
}

// ── override vs new(hiding) ────────────────────────────────────────────────

public class Base
{
    public virtual string Greet() => "Base";
}

public class OverrideChild : Base
{
    public override string Greet() => "OverrideChild";  // 다형성 O
}

public class HidingChild : Base
{
    public new string Greet() => "HidingChild";         // 다형성 X (숨김)
}

// ── Shape 추상 계층 ───────────────────────────────────────────────────────

public abstract class Shape
{
    // abstract: 자식이 반드시 구현
    public abstract double Area();
    public abstract double Perimeter();

    // Template Method 패턴: 기본 구현 제공, 자식이 필요 시 재정의
    public virtual string Describe() => $"{GetType().Name}(넓이={Area():F1})";
}

// sealed: Circle에서 더 이상 상속 불가
public sealed class Circle(double radius) : Shape
{
    public double Radius { get; } = radius;
    public override double Area()      => Math.PI * Radius * Radius;
    public override double Perimeter() => 2 * Math.PI * Radius;
}

public sealed class Rectangle(double width, double height) : Shape
{
    public double Width  { get; } = width;
    public double Height { get; } = height;
    public override double Area()      => Width * Height;
    public override double Perimeter() => 2 * (Width + Height);
}

// ── 생성자 체이닝 ─────────────────────────────────────────────────────────

public class Vehicle
{
    public string Brand { get; }

    // 정적 생성자: 클래스 처음 사용 시 한 번
    static Vehicle() { /* JIT 시점에 실행 */ }

    public Vehicle(string brand, List<string> log)
    {
        Brand = brand;
        log.Add($"[Vehicle] 생성: {brand}");
    }
}

public class Car : Vehicle
{
    public string Model { get; }

    // base(...): 부모 생성자를 먼저 호출
    public Car(string brand, string model, List<string> log)
        : base(brand, log)
    {
        Model = model;
        log.Add($"[Car] 생성: {brand} {model}");
    }
}
