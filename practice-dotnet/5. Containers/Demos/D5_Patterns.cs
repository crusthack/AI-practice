using System.Collections;
using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using System.Text;

namespace Containers.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 컬렉션 성능 패턴
//   CollectionsMarshal   — List<T>/Dictionary 내부 직접 접근 (제로 카피)
//   ObjectPool<T>        — ConcurrentBag 기반 객체 풀
//   LazyCollection<T>    — 지연 초기화 컬렉션
//   BitArray             — 비트 단위 플래그 컬렉션
//   EqualityComparer<T>  — 커스텀 동등성 + HashCode 조합
//   ListDictionary       — System.Collections.Specialized 특수 컬렉션
// ────────────────────────────────────────────────────────────────────────────
static class D5_Patterns
{
    public static void Run()
    {
        Print.Header("5. 컬렉션 성능 & 유틸리티 패턴");

        ShowCollectionsMarshal();
        ShowObjectPool();
        ShowLazyCollection();
        ShowBitArray();
        ShowCustomEqualityComparer();
        ShowSpecializedCollections();
    }

    // ── 5-1. CollectionsMarshal ───────────────────────────────────────────
    static void ShowCollectionsMarshal()
    {
        Print.Section("5-1. CollectionsMarshal — 제로 카피 내부 접근");

        // AsSpan: List<T>의 내부 배열을 Span으로 — 복사·할당 없음
        var list = new List<int> { 1, 2, 3, 4, 5 };
        Span<int> span = CollectionsMarshal.AsSpan(list);
        span[0] = 99;
        Print.Line($"AsSpan 수정 → list[0]={list[0]}  (내부 배열 공유)");

        // 배열 채우기 (Add 없이 빠르게)
        var fast = new List<int>(capacity: 5);
        CollectionsMarshal.SetCount(fast, 5);  // Count를 직접 설정
        Span<int> fastSpan = CollectionsMarshal.AsSpan(fast);
        for (int i = 0; i < fastSpan.Length; i++) fastSpan[i] = i * i;
        Print.Items(fast, "SetCount+AsSpan 채우기: ");

        // GetValueRefOrAddDefault: Dictionary 조회+삽입 단일 연산
        var wordCount = new Dictionary<string, int>();
        string[] words = ["apple", "banana", "apple", "cherry", "banana", "apple"];
        foreach (var w in words)
        {
            ref int count = ref CollectionsMarshal.GetValueRefOrAddDefault(wordCount, w, out _);
            count++;  // 조회와 업데이트가 단일 해시 탐색으로 처리됨
        }
        foreach (var (k, v) in wordCount.OrderByDescending(x => x.Value))
            Console.Write($"      {k}:{v}  ");
        Console.WriteLine();

        // GetValueRefOrNullRef: 없으면 null ref (고성능 조건부 업데이트)
        ref int appleRef = ref CollectionsMarshal.GetValueRefOrNullRef(wordCount, "apple");
        if (!System.Runtime.CompilerServices.Unsafe.IsNullRef(ref appleRef))
            Print.Line($"apple count via ref: {appleRef}");
    }

    // ── 5-2. ObjectPool<T> ────────────────────────────────────────────────
    static void ShowObjectPool()
    {
        Print.Section("5-2. ObjectPool<T> — 객체 재사용 (ConcurrentBag 기반)");

        var pool = new SimpleObjectPool<StringBuilder>(
            factory: () => new StringBuilder(),
            reset:   sb => sb.Clear());

        // 렌트 → 사용 → 반환
        var sb1 = pool.Rent();
        sb1.Append("Hello, ").Append("World!");
        string result = sb1.ToString();
        pool.Return(sb1);
        Print.Line($"Rent → 사용: \"{result}\"  반환");
        Print.Line($"Pool 사용 가능 객체: {pool.Available}");

        // 두 번째 렌트는 재사용 객체 (새로 할당 없음)
        var sb2 = pool.Rent();
        Print.Line($"재렌트 후 pool 가용: {pool.Available}");
        sb2.Append("재사용 객체 동작 확인: ").Append(sb2.Length == 0 ? "초기화됨" : "미초기화");
        Print.Line(sb2.ToString());
        pool.Return(sb2);

        // 대량 병렬 렌트/반환
        int borrowed = 0;
        Parallel.For(0, 100, _ =>
        {
            var obj = pool.Rent();
            Interlocked.Increment(ref borrowed);
            pool.Return(obj);
        });
        Print.Line($"100회 병렬 Rent/Return 완료  최종 pool 사용 가능: {pool.Available}");
    }

    // ── 5-3. LazyCollection<T> ───────────────────────────────────────────
    static void ShowLazyCollection()
    {
        Print.Section("5-3. LazyCollection<T> — 지연 초기화 컬렉션");

        int initCount = 0;
        var lazy = new LazyCollection<int>(() =>
        {
            initCount++;
            Console.WriteLine($"      [초기화! initCount={initCount}]");
            return Enumerable.Range(1, 5).ToList();
        });

        Print.Line($"LazyCollection 생성 직후 initCount={initCount}");

        // 첫 접근 시 초기화
        int count = lazy.Count;
        Print.Line($"Count 접근 후 initCount={initCount}  Count={count}");

        // 이후 접근은 캐시 사용
        _ = lazy.Count;
        _ = lazy.Count;
        Print.Line($"재접근 후 initCount={initCount}  (초기화 1번만)");

        Print.Items(lazy, "Items: ");
    }

    // ── 5-4. BitArray ─────────────────────────────────────────────────────
    static void ShowBitArray()
    {
        Print.Section("5-4. BitArray — 비트 단위 플래그 (메모리 효율)");

        // 1000개 플래그를 BitArray로 = 125바이트 (vs bool[] = 1000바이트)
        var bits = new BitArray(10, false);
        bits[0] = bits[2] = bits[4] = bits[6] = bits[8] = true;  // 짝수 인덱스 세팅

        Print.Line($"Length={bits.Length}  [0..4]: {bits[0]},{bits[1]},{bits[2]},{bits[3]},{bits[4]}");

        // 비트 연산
        var evens = new BitArray(10); evens[0] = evens[2] = evens[4] = evens[6] = evens[8] = true;
        var low5  = new BitArray(10); for (int i = 0; i < 5; i++) low5[i] = true;

        var andBits = (BitArray)evens.Clone(); andBits.And(low5);
        var orBits  = (BitArray)evens.Clone(); orBits.Or(low5);
        var xorBits = (BitArray)evens.Clone(); xorBits.Xor(low5);
        var notBits = (BitArray)evens.Clone(); notBits.Not();

        Print.Line($"evens AND low5: {BitsToString(andBits)}");
        Print.Line($"evens OR  low5: {BitsToString(orBits)}");
        Print.Line($"evens XOR low5: {BitsToString(xorBits)}");
        Print.Line($"NOT evens:      {BitsToString(notBits)}");

        // 에라토스테네스의 체 — BitArray 활용
        var sieve = SieveOfEratosthenes(50);
        Print.Items(sieve, "소수(≤50): ");
    }

    static string BitsToString(BitArray b)
    {
        var sb = new System.Text.StringBuilder();
        for (int i = 0; i < b.Length; i++) sb.Append(b[i] ? '1' : '0');
        return sb.ToString();
    }

    static IEnumerable<int> SieveOfEratosthenes(int limit)
    {
        var composite = new BitArray(limit + 1);
        for (int i = 2; i * i <= limit; i++)
            if (!composite[i])
                for (int j = i * i; j <= limit; j += i)
                    composite[j] = true;
        return Enumerable.Range(2, limit - 1).Where(n => !composite[n]);
    }

    // ── 5-5. 커스텀 EqualityComparer ─────────────────────────────────────
    static void ShowCustomEqualityComparer()
    {
        Print.Section("5-5. 커스텀 EqualityComparer<T> — HashSet/Dictionary 동등성");

        // IEqualityComparer<T> 구현체 주입
        var set = new HashSet<Point3D>(new Point3DEqualityComparer(epsilon: 0.001));
        set.Add(new Point3D(1.0, 2.0, 3.0));
        set.Add(new Point3D(1.0001, 2.0, 3.0));  // epsilon 내 → 중복
        set.Add(new Point3D(1.5,   2.0, 3.0));

        Print.Line($"HashSet Count={set.Count}  (1.0001 ≈ 1.0, epsilon=0.001)");

        // EqualityComparer<T>.Create — 람다로 즉석 생성 (.NET 7+)
        var byLen = EqualityComparer<string>.Create(
            (a, b) => (a?.Length ?? 0) == (b?.Length ?? 0),
            s => s?.Length ?? 0);
        var lenSet = new HashSet<string>(byLen) { "abc", "def", "ab", "z" };
        Print.Line($"길이 기준 HashSet Count={lenSet.Count}  (abc=def=3자)");

        // GroupBy with comparer
        var words = new[] { "Apple", "APPLE", "banana", "Banana" };
        var groups = words.GroupBy(w => w, StringComparer.OrdinalIgnoreCase).ToList();
        foreach (var g in groups)
            Print.Line($"OrdinalIgnoreCase group [{g.Key}]: {string.Join(",", g)}");
    }

    // ── 5-6. 특수 컬렉션 ────────────────────────────────────────────────
    static void ShowSpecializedCollections()
    {
        Print.Section("5-6. 특수 컬렉션 — ConcurrentBag·BlockingCollection·Channel");

        // ConcurrentBag: 순서 없는 스레드 안전 컬렉션
        var bag = new ConcurrentBag<int>();
        Parallel.For(0, 10, i => bag.Add(i));
        Print.Line($"ConcurrentBag Parallel.Add(0~9): Count={bag.Count}");

        // BlockingCollection: 생산자-소비자 경계
        var bc = new BlockingCollection<string>(boundedCapacity: 3);
        Task.Run(() => { foreach (var s in new[] { "A", "B", "C" }) { bc.Add(s); } bc.CompleteAdding(); });
        var consumed = new List<string>();
        foreach (var item in bc.GetConsumingEnumerable()) consumed.Add(item);
        Print.Items(consumed, "BlockingCollection 소비: ");

        // ConcurrentDictionary GetOrAdd + AddOrUpdate
        var cd = new ConcurrentDictionary<string, int>();
        Parallel.For(0, 100, _ => cd.AddOrUpdate("count", 1, (_, old) => old + 1));
        Print.Line($"ConcurrentDictionary AddOrUpdate 100회: count={cd["count"]}");

        // ConcurrentQueue
        var cq = new ConcurrentQueue<int>();
        Parallel.For(0, 5, i => cq.Enqueue(i));
        var results = new List<int>();
        while (cq.TryDequeue(out int v)) results.Add(v);
        results.Sort();
        Print.Items(results, "ConcurrentQueue: ");
    }
}

// ── ObjectPool<T> ─────────────────────────────────────────────────────────

public sealed class SimpleObjectPool<T>
{
    private readonly ConcurrentBag<T> _pool = new();
    private readonly Func<T>          _factory;
    private readonly Action<T>        _reset;
    private int _available;

    public SimpleObjectPool(Func<T> factory, Action<T>? reset = null)
    {
        _factory   = factory;
        _reset     = reset ?? (_ => { });
    }

    public int Available => _available;

    public T Rent()
    {
        if (_pool.TryTake(out T? item))
        {
            Interlocked.Decrement(ref _available);
            return item;
        }
        return _factory();
    }

    public void Return(T item)
    {
        _reset(item);
        _pool.Add(item);
        Interlocked.Increment(ref _available);
    }
}

// ── LazyCollection<T> ────────────────────────────────────────────────────

public sealed class LazyCollection<T> : IReadOnlyList<T>
{
    private readonly Lazy<IReadOnlyList<T>> _lazy;

    public LazyCollection(Func<IReadOnlyList<T>> initializer)
        => _lazy = new Lazy<IReadOnlyList<T>>(initializer);

    public T    this[int i] => _lazy.Value[i];
    public int  Count       => _lazy.Value.Count;
    public bool IsValueCreated => _lazy.IsValueCreated;

    public IEnumerator<T> GetEnumerator() => _lazy.Value.GetEnumerator();
    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
}

// ── Point3D + EqualityComparer ────────────────────────────────────────────

public record struct Point3D(double X, double Y, double Z);

public sealed class Point3DEqualityComparer(double epsilon) : IEqualityComparer<Point3D>
{
    public bool Equals(Point3D a, Point3D b) =>
        Math.Abs(a.X - b.X) <= epsilon &&
        Math.Abs(a.Y - b.Y) <= epsilon &&
        Math.Abs(a.Z - b.Z) <= epsilon;

    public int GetHashCode(Point3D p) =>
        HashCode.Combine(
            (int)(p.X / epsilon),
            (int)(p.Y / epsilon),
            (int)(p.Z / epsilon));
}
