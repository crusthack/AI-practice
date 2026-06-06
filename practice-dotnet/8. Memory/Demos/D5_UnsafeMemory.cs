using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Memory.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 비관리 메모리 & unsafe 직접 제어
//   fixed/포인터 산술, stackalloc, Marshal.AllocHGlobal/FreeHGlobal,
//   NativeMemory (.NET 6+), Unsafe 클래스, 구조체 레이아웃 제어
// ────────────────────────────────────────────────────────────────────────────
static class D5_UnsafeMemory
{
    public static void Run()
    {
        Print.Header("5. unsafe & 비관리 메모리 직접 제어");

        ShowFixed();
        ShowMarshalAlloc();
        ShowNativeMemory();
        ShowUnsafeHelpers();
        ShowStructLayout();
    }

    // ── 5-1. fixed & 포인터 산술 ─────────────────────────────────────────────
    static unsafe void ShowFixed()
    {
        Print.Section("5-1. fixed 문 & 포인터 산술");

        int[] arr = { 1, 2, 3, 4, 5 };

        fixed (int* p = arr)
        {
            Print.Line($"배열 고정 주소: 0x{(nint)p:X}");
            Print.Line($"p[0]={p[0]}  *(p+1)={*(p + 1)}");
            *(p + 2) = 99; // 포인터로 직접 쓰기
            Print.Line($"*(p+2)=99 → arr[2]={arr[2]}");

            // 포인터 순회 합산
            long sum = 0;
            int* cur = p;
            for (int i = 0; i < arr.Length; i++) sum += *cur++;
            Print.Line($"포인터 순회 합계: {sum}");
        }

        // stackalloc — Span<T>로 래핑 (unsafe 없이도 가능하지만 여기선 unsafe 컨텍스트)
        Span<int> stk = stackalloc int[8];
        for (int i = 0; i < stk.Length; i++) stk[i] = i * i;
        Print.Items(stk.ToArray(), "stackalloc[8]: ");

        // stackalloc + fixed
        fixed (int* sp = stk)
            Print.Line($"stackalloc 주소: 0x{(nint)sp:X}  [4]={sp[4]}");
    }

    // ── 5-2. Marshal.AllocHGlobal ────────────────────────────────────────────
    static void ShowMarshalAlloc()
    {
        Print.Section("5-2. Marshal.AllocHGlobal — 비관리(네이티브) 힙 할당");

        const int count = 8;
        IntPtr ptr = Marshal.AllocHGlobal(count * sizeof(int));
        try
        {
            for (int i = 0; i < count; i++)
                Marshal.WriteInt32(ptr, i * sizeof(int), i * i);

            var read = new int[count];
            for (int i = 0; i < count; i++)
                read[i] = Marshal.ReadInt32(ptr, i * sizeof(int));

            Print.Items(read, "비관리 힙 읽기: ");
        }
        finally
        {
            Marshal.FreeHGlobal(ptr);
            Print.Line("FreeHGlobal 완료");
        }

        // 문자열 ↔ 비관리 메모리 Marshal
        IntPtr strPtr = Marshal.StringToHGlobalUni("Hello, Native!");
        try
        {
            string? back = Marshal.PtrToStringUni(strPtr);
            Print.Line($"Marshal 문자열 왕복: \"{back}\"");
        }
        finally { Marshal.FreeHGlobal(strPtr); }
    }

    // ── 5-3. NativeMemory (.NET 6+) ──────────────────────────────────────────
    static unsafe void ShowNativeMemory()
    {
        Print.Section("5-3. NativeMemory — .NET 6+ 저수준 메모리 API");

        // Alloc (초기화 없음, 빠름)
        const int n = 6;
        int* intPtr = (int*)NativeMemory.Alloc((nuint)(n * sizeof(int)));
        try
        {
            for (int i = 0; i < n; i++) intPtr[i] = (i + 1) * 10;
            var span = new Span<int>(intPtr, n);
            Print.Items(span.ToArray(), "NativeMemory.Alloc: ");
        }
        finally { NativeMemory.Free(intPtr); }

        // AllocZeroed — 0으로 초기화 보장
        byte* zeroed = (byte*)NativeMemory.AllocZeroed(8);
        try
        {
            bool allZero = new Span<byte>(zeroed, 8).ToArray().All(b => b == 0);
            Print.Line($"AllocZeroed 전부 0: {allZero}");
        }
        finally { NativeMemory.Free(zeroed); }

        // AlignedAlloc — SIMD 등 정렬 필요 시
        float* aligned = (float*)NativeMemory.AlignedAlloc(4 * sizeof(float), alignment: 16);
        try
        {
            aligned[0] = 1.0f; aligned[1] = 2.0f; aligned[2] = 3.0f; aligned[3] = 4.0f;
            Print.Line($"AlignedAlloc(16): [{aligned[0]}, {aligned[1]}, {aligned[2]}, {aligned[3]}]");
            Print.Line($"16바이트 정렬 확인: {(nint)aligned % 16 == 0}");
        }
        finally { NativeMemory.AlignedFree(aligned); }

        // Realloc — 크기 변경 (realloc과 동일)
        byte* buf = (byte*)NativeMemory.Alloc(4);
        for (int i = 0; i < 4; i++) buf[i] = (byte)(i + 1);
        buf = (byte*)NativeMemory.Realloc(buf, 8);
        buf[4] = 5; buf[5] = 6; buf[6] = 7; buf[7] = 8;
        Print.Items(new Span<byte>(buf, 8).ToArray(), "Realloc 4→8: ");
        NativeMemory.Free(buf);
    }

    // ── 5-4. System.Runtime.CompilerServices.Unsafe ──────────────────────────
    static unsafe void ShowUnsafeHelpers()
    {
        Print.Section("5-4. Unsafe — 저수준 재해석·포인터 산술");

        // SizeOf<T>: IL 수준 크기 (패딩 포함 인라인 크기)
        Print.Line($"SizeOf: int={Unsafe.SizeOf<int>()}  double={Unsafe.SizeOf<double>()}  Guid={Unsafe.SizeOf<Guid>()}");

        // Unsafe.As<T,U>: 재해석 (비트 변환, GC 참여 없음)
        float f = 1.0f;
        int bits = Unsafe.As<float, int>(ref f);
        Print.Line($"float 1.0f 비트: 0x{bits:X8}  (IEEE 754 기댓값: 0x3F800000)");

        // Unsafe.Add — ref 포인터 산술
        int[] arr = { 10, 20, 30, 40, 50 };
        ref int r = ref arr[0];
        ref int r3 = ref Unsafe.Add(ref r, 3);
        Print.Line($"Unsafe.Add(ref arr[0], 3) = {r3}");  // 40

        // Unsafe.CopyBlock
        byte[] src = { 1, 2, 3, 4, 5 };
        byte[] dst = new byte[5];
        fixed (byte* ps = src, pd = dst)
            Unsafe.CopyBlock(pd, ps, (uint)src.Length);
        Print.Items(dst, "CopyBlock: ");

        // Unsafe.InitBlock
        byte[] buf2 = new byte[6];
        fixed (byte* pb = buf2)
            Unsafe.InitBlock(pb, 0xCC, (uint)buf2.Length);
        Print.Items(buf2, $"InitBlock(0xCC): ");

        // Unsafe.IsNullRef — null ref 검사
        int[] empty = [];
        ref int nullish = ref MemoryMarshal.GetReference(empty.AsSpan());
        Print.Line($"빈 Span GetReference IsNullRef: {Unsafe.IsNullRef(ref nullish)}");
    }

    // ── 5-5. 구조체 메모리 레이아웃 ────────────────────────────────────────
    static unsafe void ShowStructLayout()
    {
        Print.Section("5-5. StructLayout — Sequential·Explicit·Pack");

        // Sequential (기본 정렬)
        Print.Line($"SeqStruct  Marshal.SizeOf={Marshal.SizeOf<SeqStruct>()}");
        Print.Line($"  offsets: b={Marshal.OffsetOf<SeqStruct>(nameof(SeqStruct.B))}  i={Marshal.OffsetOf<SeqStruct>(nameof(SeqStruct.I))}  s={Marshal.OffsetOf<SeqStruct>(nameof(SeqStruct.S))}");

        // Pack=1 — 패딩 없음
        Print.Line($"PackedStruct(Pack=1)  Marshal.SizeOf={Marshal.SizeOf<PackedStruct>()}  (vs Sequential={Marshal.SizeOf<SeqStruct>()})");

        // Explicit — Union 시뮬레이션
        var u = new UnionStruct { Int32Value = 0x12345678 };
        Print.Line($"UnionStruct 0x{u.Int32Value:X8}: Byte0=0x{u.Byte0:X2}  Byte1=0x{u.Byte1:X2}  (리틀 엔디안)");

        // sizeof vs Marshal.SizeOf vs Unsafe.SizeOf
        Print.Line($"sizeof(int)={sizeof(int)}  Unsafe.SizeOf<int>={Unsafe.SizeOf<int>()}  Marshal.SizeOf<int>={Marshal.SizeOf<int>()}");
    }
}

// ── 구조체 레이아웃 정의 (public → Tests 접근 가능) ─────────────────────────

[StructLayout(LayoutKind.Sequential)]
public struct SeqStruct
{
    public byte  B;   // offset  0  (1B)
    // padding 3B
    public int   I;   // offset  4  (4B)
    public short S;   // offset  8  (2B)
    // padding 2B → total 12B
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct PackedStruct
{
    public byte  B;  // offset 0
    public int   I;  // offset 1 (no padding)
    public short S;  // offset 5 → total 7B
}

[StructLayout(LayoutKind.Explicit)]
public struct UnionStruct
{
    [FieldOffset(0)] public int   Int32Value;
    [FieldOffset(0)] public byte  Byte0;
    [FieldOffset(1)] public byte  Byte1;
    [FieldOffset(2)] public byte  Byte2;
    [FieldOffset(3)] public byte  Byte3;
}
