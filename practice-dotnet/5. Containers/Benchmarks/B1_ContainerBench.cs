using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using Containers.Demos;
using System.Buffers;
using System.Collections.Frozen;
using System.Runtime.InteropServices;

namespace ContBench;

// ── 컨테이너 성능 벤치마크 ──────────────────────────────────────────────
// 1. 조회: Dictionary vs FrozenDictionary vs SortedDictionary
// 2. 할당: new int[] vs ArrayPool.Rent/Return
// 3. 접근: Array indexer vs Span indexer vs List indexer
// 4. 스택: System.Stack<T> vs 직접 구현 ArrayStack<T>
// 5. CollectionsMarshal.AsSpan vs foreach
[ShortRunJob]
[MemoryDiagnoser]
public class B1_ContainerBench
{
    private const int N = 10_000;

    private Dictionary<int, int>       _dict     = null!;
    private FrozenDictionary<int, int>  _frozen   = null!;
    private SortedDictionary<int, int>  _sorted   = null!;
    private int[]                       _array    = null!;
    private List<int>                   _list     = null!;
    private Stack<int>                  _sysStack = null!;

    [GlobalSetup]
    public void Setup()
    {
        var pairs = Enumerable.Range(0, N).Select(i => (i, i * 2)).ToArray();
        _dict   = pairs.ToDictionary(p => p.i, p => p.Item2);
        _frozen = _dict.ToFrozenDictionary();
        _sorted = new SortedDictionary<int, int>(_dict);
        _array  = Enumerable.Range(0, N).ToArray();
        _list   = new List<int>(_array);
        _sysStack = new Stack<int>(_array);
    }

    // ── 딕셔너리 조회 ─────────────────────────────────────────────────────

    [Benchmark(Baseline = true)]
    public int Dictionary_Lookup()
    {
        int sum = 0;
        for (int i = 0; i < N; i++) sum += _dict[i];
        return sum;
    }

    [Benchmark]
    public int FrozenDictionary_Lookup()
    {
        int sum = 0;
        for (int i = 0; i < N; i++) sum += _frozen[i];
        return sum;
    }

    [Benchmark]
    public int SortedDictionary_Lookup()
    {
        int sum = 0;
        for (int i = 0; i < N; i++) sum += _sorted[i];
        return sum;
    }

    // ── 배열 할당 ─────────────────────────────────────────────────────────

    [Benchmark]
    public int NewArray_AllocAndSum()
    {
        var arr = new int[N];
        arr.AsSpan().Fill(1);
        int sum = 0;
        foreach (int x in arr) sum += x;
        return sum;
    }

    [Benchmark]
    public int ArrayPool_RentAndSum()
    {
        int[] arr = ArrayPool<int>.Shared.Rent(N);
        try
        {
            arr.AsSpan(0, N).Fill(1);
            int sum = 0;
            foreach (int x in arr.AsSpan(0, N)) sum += x;
            return sum;
        }
        finally { ArrayPool<int>.Shared.Return(arr); }
    }

    // ── 접근 패턴 ─────────────────────────────────────────────────────────

    [Benchmark]
    public long Array_Indexer()
    {
        long sum = 0;
        for (int i = 0; i < _array.Length; i++) sum += _array[i];
        return sum;
    }

    [Benchmark]
    public long Span_Foreach()
    {
        long sum = 0;
        foreach (int x in _array.AsSpan()) sum += x;
        return sum;
    }

    [Benchmark]
    public long List_Foreach_Via_Span()
    {
        long sum = 0;
        foreach (int x in CollectionsMarshal.AsSpan(_list)) sum += x;
        return sum;
    }

    [Benchmark]
    public long List_Foreach()
    {
        long sum = 0;
        foreach (int x in _list) sum += x;
        return sum;
    }

    // ── 스택 비교 ─────────────────────────────────────────────────────────

    [Benchmark]
    public int SystemStack_PushPop()
    {
        var s = new Stack<int>(N);
        for (int i = 0; i < N; i++) s.Push(i);
        int sum = 0;
        while (s.Count > 0) sum += s.Pop();
        return sum;
    }

    [Benchmark]
    public int ArrayStack_PushPop()
    {
        var s = new ArrayStack<int>(N);
        for (int i = 0; i < N; i++) s.Push(i);
        int sum = 0;
        while (s.Count > 0) sum += s.Pop();
        return sum;
    }
}
