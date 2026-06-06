using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using System.Collections.Immutable;

namespace IterBench;

// ── 컬렉션 연산 벤치마크 ─────────────────────────────────────────────────
// 목표: Array / List<T> / LinkedList<T> / ImmutableList<T> 연산 성능 비교
// 예상:
//   순차 읽기: Array ≈ List >> LinkedList (캐시 미스)
//   임의 접근: Array = List  >> LinkedList (O(n) 탐색)
//   삽입(앞):  LinkedList(O(1)) >> List(O(n)) >> ImmutableList(O(log n))
//   추가(뒤):  Array(N/A) ≈ List(O(1) amortized) < LinkedList
[ShortRunJob]
[MemoryDiagnoser]
public class B2_CollectionOps
{
    private const int N = 1_000;

    private int[] _array = null!;
    private List<int> _list = null!;
    private LinkedList<int> _linked = null!;
    private ImmutableList<int> _immutable = null!;
    private ImmutableArray<int> _immutableArr;

    [GlobalSetup]
    public void Setup()
    {
        var data = Enumerable.Range(0, N).ToArray();
        _array       = data;
        _list        = new List<int>(data);
        _linked      = new LinkedList<int>(data);
        _immutable   = ImmutableList.CreateRange(data);
        _immutableArr = ImmutableArray.CreateRange(data);
    }

    // ── 순차 읽기 (Sequential Read) ─────────────────────────────────────

    [Benchmark(Baseline = true)]
    public long Array_SequentialRead()
    {
        long sum = 0;
        for (int i = 0; i < _array.Length; i++) sum += _array[i];
        return sum;
    }

    [Benchmark]
    public long List_SequentialRead()
    {
        long sum = 0;
        for (int i = 0; i < _list.Count; i++) sum += _list[i];
        return sum;
    }

    [Benchmark]
    public long LinkedList_SequentialRead()
    {
        long sum = 0;
        var node = _linked.First;
        while (node is not null) { sum += node.Value; node = node.Next; }
        return sum;
    }

    [Benchmark]
    public long ImmutableList_SequentialRead()
    {
        long sum = 0;
        for (int i = 0; i < _immutable.Count; i++) sum += _immutable[i];
        return sum;
    }

    [Benchmark]
    public long ImmutableArray_SequentialRead()
    {
        long sum = 0;
        for (int i = 0; i < _immutableArr.Length; i++) sum += _immutableArr[i];
        return sum;
    }

    // ── 임의 접근 (Random Access) ────────────────────────────────────────

    [Benchmark]
    public int Array_RandomAccess()    => _array[N / 2];

    [Benchmark]
    public int List_RandomAccess()     => _list[N / 2];

    [Benchmark]
    public int ImmutableArray_RandomAccess() => _immutableArr[N / 2];

    // ── 앞에 삽입 (Insert at Front) ──────────────────────────────────────
    // 원본을 보존하기 위해 매번 복사본 사용

    [Benchmark]
    public int List_InsertFront()
    {
        var copy = new List<int>(_list);
        for (int i = 0; i < 10; i++) copy.Insert(0, i);
        return copy.Count;
    }

    [Benchmark]
    public int LinkedList_AddFirst()
    {
        var copy = new LinkedList<int>(_linked);
        for (int i = 0; i < 10; i++) copy.AddFirst(i);
        return copy.Count;
    }

    [Benchmark]
    public int ImmutableList_InsertFront()
    {
        var cur = _immutable;
        for (int i = 0; i < 10; i++) cur = cur.Insert(0, i);
        return cur.Count;
    }

    // ── 뒤에 추가 (Add at Back) ─────────────────────────────────────────

    [Benchmark]
    public int List_AddBack()
    {
        var copy = new List<int>(_list);
        for (int i = 0; i < 10; i++) copy.Add(i);
        return copy.Count;
    }

    [Benchmark]
    public int LinkedList_AddLast()
    {
        var copy = new LinkedList<int>(_linked);
        for (int i = 0; i < 10; i++) copy.AddLast(i);
        return copy.Count;
    }

    [Benchmark]
    public int ImmutableList_AddBack()
    {
        var cur = _immutable;
        for (int i = 0; i < 10; i++) cur = cur.Add(i);
        return cur.Count;
    }

    [Benchmark]
    public int ImmutableArray_AddBack()
    {
        var cur = _immutableArr;
        for (int i = 0; i < 10; i++) cur = cur.Add(i);
        return cur.Length;
    }

    // ── HashSet vs List 검색 ─────────────────────────────────────────────

    private HashSet<int> _hashSet = null!;

    [GlobalSetup(Target = nameof(HashSet_Contains))]
    public void SetupHashSet()
    {
        Setup();
        _hashSet = new HashSet<int>(_array);
    }

    [Benchmark]
    public bool List_Contains()    => _list.Contains(N / 2);

    [Benchmark]
    public bool HashSet_Contains() => _hashSet.Contains(N / 2);
}
