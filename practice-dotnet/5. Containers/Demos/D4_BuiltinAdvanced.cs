using System.Collections.Frozen;
using System.Collections.ObjectModel;

namespace Containers.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 내장 고급 컬렉션
//   SortedSet<T>             — 자가 균형 BST (범위 질의·집합 연산)
//   FrozenDictionary<K,V>    — 읽기 전용 최적화 딕셔너리 (.NET 8+)
//   FrozenSet<T>             — 읽기 전용 최적화 집합 (.NET 8+)
//   OrderedDictionary<K,V>   — 삽입 순서 보존 딕셔너리 (.NET 9+)
//   MultiMap<K,V>            — Dictionary<K, List<V>> 래퍼 패턴
//   ReadOnlyCollection<T>    — 읽기 전용 래퍼
// ────────────────────────────────────────────────────────────────────────────
static class D4_BuiltinAdvanced
{
    public static void Run()
    {
        Print.Header("4. 내장 고급 컬렉션");

        ShowSortedSet();
        ShowFrozenCollections();
        ShowOrderedDictionary();
        ShowMultiMap();
        ShowReadOnlyWrappers();
    }

    // ── 4-1. SortedSet<T> ────────────────────────────────────────────────────
    static void ShowSortedSet()
    {
        Print.Section("4-1. SortedSet<T> — 자가 균형 BST (Red-Black Tree)");

        var ss = new SortedSet<int> { 5, 2, 8, 1, 9, 3, 7, 4, 6 };
        Print.Items(ss, "SortedSet: ");
        Print.Line($"Min={ss.Min}  Max={ss.Max}  Count={ss.Count}");

        // 범위 질의 (O(log n) + 결과 수)
        var view = ss.GetViewBetween(3, 7);
        Print.Items(view, "GetViewBetween(3,7): ");

        // Reverse
        Print.Items(ss.Reverse(), "Reverse: ");

        // 집합 연산 (원본 수정)
        var a = new SortedSet<int> { 1, 2, 3, 4, 5 };
        var b = new SortedSet<int> { 3, 4, 5, 6, 7 };
        var union = new SortedSet<int>(a); union.UnionWith(b);
        var inter = new SortedSet<int>(a); inter.IntersectWith(b);
        Print.Items(union, "Union:     ");
        Print.Items(inter, "Intersect: ");

        // 커스텀 comparer (문자열 길이 기준)
        var byLen = new SortedSet<string>(Comparer<string>.Create((x, y) =>
            x.Length != y.Length ? x.Length.CompareTo(y.Length) : string.Compare(x, y, StringComparison.Ordinal)));
        byLen.Add("banana"); byLen.Add("fig"); byLen.Add("apple"); byLen.Add("kiwi"); byLen.Add("cherry");
        Print.Items(byLen, "길이 기준 정렬: ");
    }

    // ── 4-2. FrozenDictionary / FrozenSet ─────────────────────────────────
    static void ShowFrozenCollections()
    {
        Print.Section("4-2. FrozenDictionary & FrozenSet — 읽기 전용 최적화 (.NET 8+)");

        // FrozenDictionary: 구성 후 불변 → 조회 성능 최적화
        var data = new Dictionary<string, int>
        {
            ["one"] = 1, ["two"] = 2, ["three"] = 3, ["four"] = 4, ["five"] = 5,
        };
        FrozenDictionary<string, int> fd = data.ToFrozenDictionary();

        Print.Line($"FrozenDictionary Count: {fd.Count}");
        Print.Line($"fd[\"three\"]: {fd["three"]}");
        Print.Line($"TryGetValue(\"six\"): {fd.TryGetValue("six", out _)}");

        // FrozenDictionary.Keys / Values: ReadOnlySpan 접근 가능
        Print.Items(fd.Keys, "Keys: ");

        // FrozenSet
        var primes = new HashSet<int> { 2, 3, 5, 7, 11, 13, 17, 19 };
        FrozenSet<int> fs = primes.ToFrozenSet();
        Print.Line($"FrozenSet Count: {fs.Count}  Contains(7): {fs.Contains(7)}  Contains(9): {fs.Contains(9)}");

        // StringComparison 전용 FrozenDictionary (대소문자 무시)
        var ci = new Dictionary<string, string>
        {
            ["hello"] = "안녕", ["world"] = "세계"
        }.ToFrozenDictionary(StringComparer.OrdinalIgnoreCase);
        Print.Line($"OrdinalIgnoreCase fd[\"HELLO\"]: {ci["HELLO"]}");
    }

    // ── 4-3. OrderedDictionary<K,V> ──────────────────────────────────────
    static void ShowOrderedDictionary()
    {
        Print.Section("4-3. OrderedDictionary<K,V> — 삽입 순서 보존 (.NET 9+)");

        var od = new System.Collections.Generic.OrderedDictionary<string, int>();
        od.Add("banana", 2);
        od.Add("apple",  1);
        od.Add("cherry", 3);

        // 삽입 순서 보존
        Print.Items(od.Keys, "삽입 순서: ");

        // 인덱스 접근
        Print.Line($"GetAt(0): {od.GetAt(0)}");
        Print.Line($"IndexOf(\"apple\"): {od.IndexOf("apple")}");

        // 값 업데이트
        od["apple"] = 99;
        Print.Line($"Update apple=99: {od["apple"]}");

        // 인덱스로 제거
        od.RemoveAt(0);
        Print.Items(od.Keys, "RemoveAt(0) 후: ");

        // 일반 Dictionary vs OrderedDictionary 차이
        var regular = new Dictionary<string, int> { ["b"] = 2, ["a"] = 1, ["c"] = 3 };
        var ordered = new System.Collections.Generic.OrderedDictionary<string, int>
            { ["b"] = 2, ["a"] = 1, ["c"] = 3 };
        Print.Items(regular.Keys, "Dictionary(순서 미보장): ");
        Print.Items(ordered.Keys, "OrderedDictionary(순서 보장): ");
    }

    // ── 4-4. MultiMap 패턴 ───────────────────────────────────────────────
    static void ShowMultiMap()
    {
        Print.Section("4-4. MultiMap<K,V> — 한 키에 여러 값");

        var map = new MultiMap<string, string>();
        map.Add("과일", "사과");
        map.Add("과일", "바나나");
        map.Add("과일", "체리");
        map.Add("채소", "당근");
        map.Add("채소", "시금치");

        Print.Line($"Count: {map.Count}  Keys: {string.Join(",", map.Keys)}");
        Print.Items(map["과일"],  "map[\"과일\"]: ");
        Print.Items(map["채소"],  "map[\"채소\"]: ");
        Print.Line($"ContainsValue(\"과일\",\"바나나\"): {map.ContainsValue("과일", "바나나")}");

        map.Remove("과일", "바나나");
        Print.Items(map["과일"], "Remove(바나나) 후: ");

        // LINQ와의 조합
        var groups = new[] { ("A", 1), ("B", 2), ("A", 3), ("B", 4), ("C", 5) }
            .ToMultiMap(t => t.Item1, t => t.Item2);
        Print.Line($"ToMultiMap: A={string.Join(",", groups["A"])}  B={string.Join(",", groups["B"])}");
    }

    // ── 4-5. ReadOnly 래퍼들 ─────────────────────────────────────────────
    static void ShowReadOnlyWrappers()
    {
        Print.Section("4-5. ReadOnly 래퍼 — List·Dictionary·Collection");

        var list = new List<int> { 1, 2, 3, 4, 5 };
        ReadOnlyCollection<int> roList = list.AsReadOnly();
        Print.Line($"AsReadOnly: [{string.Join(",", roList)}]  Count={roList.Count}");
        list.Add(6);  // 원본 수정 → 래퍼에도 반영
        Print.Line($"원본 Add(6) 후 래퍼: Count={roList.Count}");

        // ReadOnlyDictionary
        var dict = new Dictionary<string, int> { ["a"] = 1, ["b"] = 2 };
        var roDict = new ReadOnlyDictionary<string, int>(dict);
        Print.Line($"ReadOnlyDictionary[\"a\"]: {roDict["a"]}");

        // IReadOnlyList / IReadOnlyDictionary 인터페이스
        IReadOnlyList<int>              irl = list;
        IReadOnlyDictionary<string,int> ird = dict;
        Print.Line($"IReadOnlyList[0]: {irl[0]}  IReadOnlyDictionary[\"b\"]: {ird["b"]}");

        // Collection<T>: List<T>보다 커스터마이즈 가능 (가상 메서드)
        var col = new ObservableCollection(new List<string> { "x", "y" });
        col.Add("z");
        Print.Items(col, "ObservableCollection: ");
    }

    // 간단한 ObservableCollection 대체 (변경 추적 예시)
    sealed class ObservableCollection(List<string> inner) : Collection<string>(inner)
    {
        protected override void InsertItem(int index, string item)
        {
            Console.WriteLine($"      [삽입] index={index}, item={item}");
            base.InsertItem(index, item);
        }
    }
}

// ── MultiMap<K,V> ─────────────────────────────────────────────────────────

public sealed class MultiMap<TKey, TValue> where TKey : notnull
{
    private readonly Dictionary<TKey, List<TValue>> _inner = new();

    public int Count => _inner.Count;
    public IEnumerable<TKey> Keys => _inner.Keys;

    public IReadOnlyList<TValue> this[TKey key] =>
        _inner.TryGetValue(key, out var list) ? list : Array.Empty<TValue>();

    public void Add(TKey key, TValue value)
    {
        if (!_inner.TryGetValue(key, out var list))
            _inner[key] = list = new List<TValue>();
        list.Add(value);
    }

    public bool Remove(TKey key, TValue value) =>
        _inner.TryGetValue(key, out var list) && list.Remove(value);

    public bool ContainsValue(TKey key, TValue value) =>
        _inner.TryGetValue(key, out var list) && list.Contains(value);
}

// ── MultiMap 확장 메서드 ──────────────────────────────────────────────────

public static class MultiMapExtensions
{
    public static MultiMap<TKey, TValue> ToMultiMap<T, TKey, TValue>(
        this IEnumerable<T> source,
        Func<T, TKey> keySelector,
        Func<T, TValue> valueSelector) where TKey : notnull
    {
        var map = new MultiMap<TKey, TValue>();
        foreach (var item in source)
            map.Add(keySelector(item), valueSelector(item));
        return map;
    }
}
