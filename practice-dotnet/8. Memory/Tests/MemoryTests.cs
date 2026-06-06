using Memory.Demos;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Xunit;

// ── GC 세대 & WeakReference ────────────────────────────────────────────────
public class GCTests
{
    [Fact] public void SmallObj_StartsGen0()
    {
        var o = new object();
        Assert.Equal(0, GC.GetGeneration(o));
        GC.KeepAlive(o);
    }

    [Fact] public void LOH_IsGen2()
    {
        byte[] large = new byte[85_000]; // LOH 임계값
        Assert.Equal(2, GC.GetGeneration(large));
        GC.KeepAlive(large);
    }

    [Fact] public void WeakRef_DiesAfterGC()
    {
        // 디버그 빌드에서도 강한 참조가 확실히 사라지도록 별도 메서드로 분리
        var weak = MakeWeak();
        GC.Collect(2, GCCollectionMode.Forced, blocking: true);
        GC.WaitForPendingFinalizers();
        GC.Collect(2, GCCollectionMode.Forced, blocking: true);
        Assert.False(weak.TryGetTarget(out _));
    }

    [System.Runtime.CompilerServices.MethodImpl(
        System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    static WeakReference<byte[]> MakeWeak()
        => new(new byte[1024]);

    [Fact] public void WeakRef_AliveWhileStrongRefHeld()
    {
        var obj = new object();
        var weak = new WeakReference<object>(obj);
        GC.Collect(2, GCCollectionMode.Forced, blocking: true);
        Assert.True(weak.TryGetTarget(out _));
        GC.KeepAlive(obj);
    }

    [Fact] public void POH_Array_IsGen2()
    {
        byte[] poh = GC.AllocateArray<byte>(256, pinned: true);
        Assert.Equal(2, GC.GetGeneration(poh));
        GC.KeepAlive(poh);
    }
}

// ── IDisposable ─────────────────────────────────────────────────────────────
public class DisposeTests
{
    [Fact] public void Using_CallsDispose()
    {
        ManagedResource r;
        using (r = new ManagedResource("T1")) { r.Use(); }
        Assert.True(r.IsDisposed);
    }

    [Fact] public void Use_AfterDispose_Throws()
    {
        var r = new ManagedResource("T2");
        r.Dispose();
        Assert.Throws<ObjectDisposedException>(() => r.Use());
    }

    [Fact] public void Double_Dispose_Safe()
    {
        var r = new ManagedResource("T3");
        r.Dispose();
        r.Dispose(); // 예외 없어야 함
        Assert.True(r.IsDisposed);
    }

    [Fact] public void Child_Dispose_CallsBase()
    {
        ChildResource r;
        using (r = new ChildResource("C1")) { r.Use(); }
        Assert.True(r.IsDisposed);
    }

    [Fact] public async Task AsyncDisposable_NoException()
    {
        await using var ar = new AsyncResource("AR1");
        await ar.DoWorkAsync();
    }
}

// ── Marshal & NativeMemory ──────────────────────────────────────────────────
public class NativeMemoryTests
{
    [Fact] public void Marshal_AllocFree_RoundTrip()
    {
        IntPtr p = Marshal.AllocHGlobal(4 * sizeof(int));
        try
        {
            for (int i = 0; i < 4; i++) Marshal.WriteInt32(p, i * 4, i * 100);
            Assert.Equal(0,   Marshal.ReadInt32(p, 0));
            Assert.Equal(100, Marshal.ReadInt32(p, 4));
            Assert.Equal(200, Marshal.ReadInt32(p, 8));
            Assert.Equal(300, Marshal.ReadInt32(p, 12));
        }
        finally { Marshal.FreeHGlobal(p); }
    }

    [Fact] public unsafe void NativeMemory_AllocZeroed_AllZero()
    {
        byte* ptr = (byte*)NativeMemory.AllocZeroed(16);
        try { Assert.All(new Span<byte>(ptr, 16).ToArray(), b => Assert.Equal(0, b)); }
        finally { NativeMemory.Free(ptr); }
    }

    [Fact] public unsafe void NativeMemory_AlignedAlloc()
    {
        float* p = (float*)NativeMemory.AlignedAlloc(4 * sizeof(float), alignment: 16);
        try
        {
            p[0] = 1f; p[1] = 2f;
            Assert.Equal(1f, p[0]);
            Assert.True((nint)p % 16 == 0);
        }
        finally { NativeMemory.AlignedFree(p); }
    }

    [Fact] public void Marshal_String_RoundTrip()
    {
        IntPtr p = Marshal.StringToHGlobalUni("Hello");
        try { Assert.Equal("Hello", Marshal.PtrToStringUni(p)); }
        finally { Marshal.FreeHGlobal(p); }
    }
}

// ── Unsafe 헬퍼 ─────────────────────────────────────────────────────────────
public class UnsafeTests
{
    [Fact] public void As_Float_To_Int_Bits()
    {
        float f = 1.0f;
        int bits = Unsafe.As<float, int>(ref f);
        Assert.Equal(0x3F800000, bits); // IEEE 754 1.0f
    }

    [Fact] public void Add_RefArithmetic()
    {
        int[] arr = { 10, 20, 30, 40, 50 };
        ref int r = ref Unsafe.Add(ref arr[0], 3);
        Assert.Equal(40, r);
    }

    [Fact] public unsafe void CopyBlock_Works()
    {
        byte[] src = { 1, 2, 3, 4 };
        byte[] dst = new byte[4];
        fixed (byte* ps = src, pd = dst)
            Unsafe.CopyBlock(pd, ps, 4);
        Assert.Equal(src, dst);
    }
}

// ── 구조체 레이아웃 ─────────────────────────────────────────────────────────
public class StructLayoutTests
{
    [Fact] public void Sequential_SizeOf_12()
        => Assert.Equal(12, Marshal.SizeOf<SeqStruct>());

    [Fact] public void Packed_SizeOf_7()
        => Assert.Equal(7, Marshal.SizeOf<PackedStruct>());

    [Fact] public void Union_LittleEndian_Reinterpret()
    {
        var u = new UnionStruct { Int32Value = 0x12345678 };
        Assert.Equal((byte)0x78, u.Byte0);
        Assert.Equal((byte)0x56, u.Byte1);
        Assert.Equal((byte)0x34, u.Byte2);
        Assert.Equal((byte)0x12, u.Byte3);
    }

    [Fact] public void SeqStruct_FieldOffsets()
    {
        Assert.Equal(0, (int)Marshal.OffsetOf<SeqStruct>(nameof(SeqStruct.B)));
        Assert.Equal(4, (int)Marshal.OffsetOf<SeqStruct>(nameof(SeqStruct.I)));
        Assert.Equal(8, (int)Marshal.OffsetOf<SeqStruct>(nameof(SeqStruct.S)));
    }
}
