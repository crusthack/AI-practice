using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using System.Buffers;
using System.Runtime.InteropServices;

namespace MemBench;

// ── 메모리 할당·접근 벤치마크 ──────────────────────────────────────────────
// 1. 할당 방식: new[] vs ArrayPool vs stackalloc vs Marshal vs NativeMemory
// 2. 메모리 접근: Array vs Span foreach
// 3. GC 수집 비용: Gen0 vs Gen2
// 4. 크기별 LOH 임계값 영향: SOH(1KB) vs LOH(256KB)
[ShortRunJob]
[MemoryDiagnoser]
public class B1_MemoryBench
{
    private const int N = 1024; // 1KB

    private readonly byte[] _arr = new byte[N];

    // ── 할당 비교 ─────────────────────────────────────────────────────────

    [Benchmark(Baseline = true)]
    public byte[] ManagedArray_Alloc()
    {
        byte[] buf = new byte[N];
        buf.AsSpan().Fill(0xFF);
        return buf;
    }

    [Benchmark]
    public int ArrayPool_RentReturn()
    {
        byte[] buf = ArrayPool<byte>.Shared.Rent(N);
        try   { buf.AsSpan(0, N).Fill(0xFF); return buf[0]; }
        finally { ArrayPool<byte>.Shared.Return(buf); }
    }

    [Benchmark]
    public unsafe int Stackalloc_Fill()
    {
        Span<byte> buf = stackalloc byte[64]; // 스택은 크기 제한
        buf.Fill(0xFF);
        return buf[0];
    }

    [Benchmark]
    public unsafe int MarshalAlloc_Fill()
    {
        byte* ptr = (byte*)Marshal.AllocHGlobal(N);
        try   { new Span<byte>(ptr, N).Fill(0xFF); return *ptr; }
        finally { Marshal.FreeHGlobal((IntPtr)ptr); }
    }

    [Benchmark]
    public unsafe int NativeMemory_Fill()
    {
        byte* ptr = (byte*)NativeMemory.Alloc((nuint)N);
        try   { new Span<byte>(ptr, N).Fill(0xFF); return *ptr; }
        finally { NativeMemory.Free(ptr); }
    }

    // ── LOH vs SOH ────────────────────────────────────────────────────────

    [Benchmark]
    public byte[] SOH_Alloc_1KB()  => new byte[1024];

    [Benchmark]
    public byte[] LOH_Alloc_256KB() => new byte[256 * 1024];

    // ── 접근 패턴 ─────────────────────────────────────────────────────────

    [Benchmark]
    public long Array_IndexerLoop()
    {
        long sum = 0;
        for (int i = 0; i < _arr.Length; i++) sum += _arr[i];
        return sum;
    }

    [Benchmark]
    public long Span_Foreach()
    {
        long sum = 0;
        foreach (byte b in _arr.AsSpan()) sum += b;
        return sum;
    }

    // ── GC 수집 비용 ──────────────────────────────────────────────────────

    [Benchmark]
    public void GC_Collect_Gen0()
        => GC.Collect(0, GCCollectionMode.Forced, blocking: true);

    [Benchmark]
    public void GC_Collect_Full()
        => GC.Collect(2, GCCollectionMode.Forced, blocking: true);
}
