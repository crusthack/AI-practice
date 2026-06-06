using System.Numerics;

namespace OOP.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 제네릭 (Generics)
//   제네릭 클래스·메서드 · 제약조건(constraints) · 공변/반공변 · 제네릭 수학
// ────────────────────────────────────────────────────────────────────────────
static class D4_Generics
{
    public static void Run()
    {
        Print.Header("4. 제네릭 (Generics)");

        ShowGenericClass();
        ShowConstraints();
        ShowVariance();
        ShowGenericMath();
        ShowGenericMethods();
    }

    // ── 4-1. 제네릭 클래스 ─────────────────────────────────────────────────
    static void ShowGenericClass()
    {
        Print.Section("4-1. 제네릭 클래스 — Repository<T, TKey>");

        var repo = new Repository<Product2, int>();
        repo.Add(new Product2(1, "노트북",  1_200_000m));
        repo.Add(new Product2(2, "마우스",     35_000m));
        repo.Add(new Product2(3, "키보드",    120_000m));

        Print.Line($"Count: {repo.Count}");
        Print.Line($"Get(2): {repo.Get(2)}");
        Print.Line($"Find(Price>100000): {repo.Find(p => p.Price > 100_000)}");

        repo.Update(new Product2(2, "마우스 Pro", 55_000m));
        Print.Line($"Update(2): {repo.Get(2)}");

        repo.Delete(3);
        Print.Line($"Delete(3), Count: {repo.Count}");
    }

    // ── 4-2. 제약조건 ──────────────────────────────────────────────────────
    static void ShowConstraints()
    {
        Print.Section("4-2. 제약조건 (Constraints) 총정리");

        // where T : class — 참조 형식
        Print.Line($"class  constraint: {Constraints.RefMax("apple", "banana")}");

        // where T : struct — 값 형식
        Print.Line($"struct constraint: {Constraints.Swap(10, 20)}");

        // where T : new() — 기본 생성자 있어야 함 (required 멤버 없는 타입만 가능)
        var created = Constraints.CreateNew<NewablePoint>();
        Print.Line($"new()  constraint: {created.GetType().Name} 생성됨");

        // where T : class, IComparable<T>
        Print.Line($"IComparable: {Constraints.Min(3, 7)}");
        Print.Line($"IComparable: {Constraints.Min("alpha", "beta")}");

        // where T : BaseClass
        Print.Line($"BaseClass: {Constraints.GetArea(new Circle(4))}");

        // where T : notnull (null 불허)
        var cache = new Cache<string, int>();
        cache.Set("answer", 42);
        Print.Line($"notnull Cache: {cache.Get("answer")}");

        // where T : unmanaged (관리되지 않는 값 형식, Span/포인터 연산 가능)
        Print.Line($"unmanaged Sum: {Constraints.UnmanagedSum(1, 2, 3, 4, 5)}");
    }

    // ── 4-3. 공변 / 반공변 ────────────────────────────────────────────────
    static void ShowVariance()
    {
        Print.Section("4-3. 공변(out)/반공변(in) — 인터페이스 분산");

        // 공변(covariant): out T — 더 파생된 타입을 더 기본 타입 인터페이스로 대입 가능
        IProducer<Dog2> dogProducer = new DogFactory();
        IProducer<Animal2> animalProducer = dogProducer;   // Dog2 is Animal2 → OK
        Animal2 a = animalProducer.Produce();
        Print.Line($"공변  IProducer<Dog2> → IProducer<Animal2>: {a.GetType().Name}");

        // 반공변(contravariant): in T — 기본 타입 소비자를 파생 타입 인터페이스로 대입 가능
        IConsumer<Animal2> animalConsumer = new AnimalLogger();
        IConsumer<Dog2> dogConsumer = animalConsumer;      // Animal2 handler can handle Dog2 → OK
        dogConsumer.Consume(new Dog2("멍멍이"));
        Print.Line($"반공변 IConsumer<Animal2> → IConsumer<Dog2>: 처리됨");

        // IEnumerable<out T>: 공변 (표준 라이브러리 예)
        IEnumerable<Dog2> dogs = [new Dog2("A"), new Dog2("B")];
        IEnumerable<Animal2> animals = dogs;   // 공변으로 가능
        Print.Line($"IEnumerable<Dog2> → IEnumerable<Animal2>: {animals.Count()}마리");
    }

    // ── 4-4. 제네릭 수학 (INumber<T>, .NET 7+) ────────────────────────────
    static void ShowGenericMath()
    {
        Print.Section("4-4. 제네릭 수학 — INumber<T> (System.Numerics)");

        Print.Line($"Sum<int>:    {GenericMath.Sum(1, 2, 3, 4, 5)}");
        Print.Line($"Sum<double>: {GenericMath.Sum(1.1, 2.2, 3.3, 4.4):F2}");
        Print.Line($"Sum<long>:   {GenericMath.Sum(100L, 200L, 300L)}");

        Print.Line($"Average<int>:    {GenericMath.Average(10, 20, 30, 40):F2}");
        Print.Line($"Average<double>: {GenericMath.Average(1.5, 2.5, 3.5):F2}");

        Print.Line($"Clamp<int>(15, 0, 10):    {GenericMath.Clamp(15, 0, 10)}");
        Print.Line($"Clamp<float>(5f, 0f, 3f): {GenericMath.Clamp(5f, 0f, 3f)}");
    }

    // ── 4-5. 제네릭 메서드 ─────────────────────────────────────────────────
    static void ShowGenericMethods()
    {
        Print.Section("4-5. 제네릭 메서드 — 추론·명시적 타입 인수");

        // 타입 추론 (type inference)
        int[]    ints  = [3, 1, 4, 1, 5];
        string[] words = ["banana", "apple", "cherry"];

        Print.Line($"Swap<int>:    {GenericMethods.Swap(ints, 0, 1)}  → [{string.Join(",", ints)}]");
        Print.Line($"Swap<string>: {GenericMethods.Swap(words, 0, 2)} → [{string.Join(",", words)}]");

        // Zip 변환
        var pairs = GenericMethods.ZipWith(new[] { 1, 2, 3 }, new[] { 10, 20, 30 }, (a, b) => a + b);
        Print.Line($"ZipWith(+): [{string.Join(",", pairs)}]");

        // Pipeline (함수 합성)
        var result = GenericMethods.Pipe(5, x => x * 2, x => x + 1, x => x.ToString());
        Print.Line($"Pipe(5, ×2, +1, toString): {result}");
    }
}

// ── 1. 제네릭 Repository ──────────────────────────────────────────────────

// new() 데모용 단순 클래스 (required 멤버 없음)
public class NewablePoint { public int X, Y; }

public interface IEntity<TKey>
{
    TKey Id { get; }
}

public record Product2(int Id, string Name, decimal Price) : IEntity<int>
{
    public override string ToString() => $"#{Id} {Name} {Price:C}";
}

// T : class, IEntity<TKey> — 참조 형식이고 IEntity<TKey>를 구현해야 함
public class Repository<T, TKey> where T : class, IEntity<TKey> where TKey : notnull
{
    private readonly Dictionary<TKey, T> _store = new();

    public int Count => _store.Count;

    public void Add(T item)    => _store.Add(item.Id, item);
    public void Delete(TKey id) => _store.Remove(id);
    public T?   Get(TKey id)   => _store.GetValueOrDefault(id);

    public void Update(T item)
    {
        if (_store.ContainsKey(item.Id)) _store[item.Id] = item;
    }

    public T? Find(Func<T, bool> predicate) => _store.Values.FirstOrDefault(predicate);
}

// ── 2. 제약조건 모음 ────────────────────────────────────────────────────

public static class Constraints
{
    // where T : class — 참조 형식만
    public static T RefMax<T>(T a, T b) where T : class, IComparable<T>
        => a.CompareTo(b) >= 0 ? a : b;

    // where T : struct — 값 형식만
    public static string Swap<T>(T a, T b) where T : struct
        => $"({b}, {a})";

    // where T : new() — 매개변수 없는 생성자
    public static T CreateNew<T>() where T : new() => new T();

    // where T : IComparable<T>
    public static T Min<T>(T a, T b) where T : IComparable<T>
        => a.CompareTo(b) <= 0 ? a : b;

    // where T : Shape (특정 기본 클래스)
    public static double GetArea<T>(T shape) where T : Shape
        => shape.Area();

    // where T : unmanaged (관리되지 않는 값 형식)
    public static unsafe T UnmanagedSum<T>(params T[] values)
        where T : unmanaged, INumber<T>
    {
        T sum = T.Zero;
        foreach (var v in values) sum += v;
        return sum;
    }
}

// notnull 캐시
public class Cache<TKey, TValue> where TKey : notnull
{
    private readonly Dictionary<TKey, TValue?> _cache = new();
    public void Set(TKey k, TValue v) => _cache[k] = v;
    public TValue? Get(TKey k) => _cache.TryGetValue(k, out var v) ? v : default;
}

// ── 3. 공변/반공변 ────────────────────────────────────────────────────────

public class Animal2(string name)
{
    public string Name { get; } = name;
}
public class Dog2(string name) : Animal2(name) { }

// out T: 공변 — T를 반환만 함 (더 파생된 타입으로 대입 가능)
public interface IProducer<out T>
{
    T Produce();
}

// in T: 반공변 — T를 입력으로만 받음 (더 기본 타입 소비자로 대입 가능)
public interface IConsumer<in T>
{
    void Consume(T item);
}

public class DogFactory : IProducer<Dog2>
{
    public Dog2 Produce() => new("팩토리 개");
}

public class AnimalLogger : IConsumer<Animal2>
{
    public void Consume(Animal2 a) => Console.WriteLine($"      로그: {a.GetType().Name}({a.Name})");
}

// ── 4. 제네릭 수학 ───────────────────────────────────────────────────────

public static class GenericMath
{
    // where T : INumber<T>: 더하기·비교 등 수학 연산 가능
    public static T Sum<T>(params T[] values) where T : INumber<T>
    {
        T total = T.Zero;
        foreach (var v in values) total += v;
        return total;
    }

    public static double Average<T>(params T[] values) where T : INumber<T>
    {
        double sum = double.CreateChecked(Sum(values));
        return sum / values.Length;
    }

    public static T Clamp<T>(T value, T min, T max) where T : INumber<T>
        => T.Clamp(value, min, max);
}

// ── 5. 제네릭 메서드 ─────────────────────────────────────────────────────

public static class GenericMethods
{
    // 배열 두 요소 교환 후 결과 설명 반환
    public static string Swap<T>(T[] arr, int i, int j)
    {
        (arr[i], arr[j]) = (arr[j], arr[i]);
        return $"swap({i},{j})";
    }

    // 두 시퀀스를 결합자 함수로 합치기
    public static IEnumerable<TResult> ZipWith<T1, T2, TResult>(
        IEnumerable<T1> a,
        IEnumerable<T2> b,
        Func<T1, T2, TResult> combiner)
        => a.Zip(b, combiner);

    // 함수 파이프라인 (3단계)
    public static TOut Pipe<TIn, TMid1, TMid2, TOut>(
        TIn input,
        Func<TIn,   TMid1> f1,
        Func<TMid1, TMid2> f2,
        Func<TMid2, TOut>  f3)
        => f3(f2(f1(input)));
}
