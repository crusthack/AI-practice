using System.Runtime;
using System.Runtime.InteropServices;

namespace Memory.Demos;

// ────────────────────────────────────────────────────────────────────────────
// LOH & 고정 메모리
//   LOH 임계값(85,000B), 단편화, GCLargeObjectHeapCompactionMode,
//   GCHandle (Normal/Pinned/Weak), Pinned Object Heap (POH .NET 5+)
// ────────────────────────────────────────────────────────────────────────────
static class D2_LOH
{
    // LOH 임계값: 85,000 바이트
    public const int LohThreshold = 85_000;

    public static void Run()
    {
        Print.Header("2. LOH & 고정 메모리 (GCHandle · POH)");

        ShowLohThreshold();
        ShowLohFragmentation();
        ShowGCHandle();
        ShowPinnedObjectHeap();
    }

    // ── 2-1. LOH 임계값 ──────────────────────────────────────────────────────
    static void ShowLohThreshold()
    {
        Print.Section("2-1. LOH 임계값 85,000B → Gen2 직행");

        byte[] small  = new byte[84_999];
        byte[] large  = new byte[LohThreshold];
        byte[] larger = new byte[1_000_000];

        Print.Line($"84,999B → Gen{GC.GetGeneration(small)}   (SOH)");
        Print.Line($"85,000B → Gen{GC.GetGeneration(large)}   (LOH — Gen2 직행)");
        Print.Line($"  1MB   → Gen{GC.GetGeneration(larger)}  (LOH)");

        // Gen0 수집해도 LOH 객체는 영향받지 않음
        GC.Collect(0, GCCollectionMode.Forced, blocking: true);
        Print.Line($"Gen0 수집 후 large 세대: Gen{GC.GetGeneration(large)}  (여전히 Gen2)");

        GC.KeepAlive(small); GC.KeepAlive(large); GC.KeepAlive(larger);
    }

    // ── 2-2. LOH 단편화 & 압축 ───────────────────────────────────────────────
    static void ShowLohFragmentation()
    {
        Print.Section("2-2. LOH 단편화 & CompactOnce");

        // 교대 할당/해제 → LOH 단편화 시뮬레이션
        var alive = new List<byte[]>();
        for (int i = 0; i < 12; i++)
        {
            var a = new byte[LohThreshold * (i % 3 + 1)];
            if (i % 2 == 0) alive.Add(a);  // 홀수 인덱스는 해제됨
        }

        long before = GC.GetTotalMemory(false);
        GC.Collect(2, GCCollectionMode.Forced, blocking: true);
        long afterNoCompact = GC.GetTotalMemory(false);
        Print.Line($"Gen2 수집 후(압축 없음): {afterNoCompact / 1024}KB  (단편 메모리 잔존 가능)");

        // CompactOnce: 다음 Gen2 수집 1회에 한해 LOH 압축
        GCSettings.LargeObjectHeapCompactionMode = GCLargeObjectHeapCompactionMode.CompactOnce;
        GC.Collect(2, GCCollectionMode.Forced, blocking: true);
        Print.Line($"CompactOnce 수집 후 모드: {GCSettings.LargeObjectHeapCompactionMode}  (자동 Default 복원)");
        long afterCompact = GC.GetTotalMemory(false);
        Print.Line($"압축 후: {afterCompact / 1024}KB");

        GC.KeepAlive(alive);
    }

    // ── 2-3. GCHandle ────────────────────────────────────────────────────────
    static void ShowGCHandle()
    {
        Print.Section("2-3. GCHandle — 네이티브 코드용 핸들 종류");

        byte[] data = { 10, 20, 30, 40, 50 };

        // Normal: GC가 이동시킬 수 있지만 수집은 못함
        GCHandle normal = GCHandle.Alloc(data);
        Print.Line($"Normal  IsAllocated: {normal.IsAllocated}");
        normal.Free();

        // Pinned: 메모리 위치를 고정 → 네이티브 포인터 전달 가능
        GCHandle pinned = GCHandle.Alloc(data, GCHandleType.Pinned);
        IntPtr addr = pinned.AddrOfPinnedObject();
        Print.Line($"Pinned  주소: 0x{addr:X}  [0]={Marshal.ReadByte(addr)}");
        pinned.Free();

        // Weak: GC가 수집 가능, TryGetTarget으로 생존 확인
        GCHandle weak = GCHandle.Alloc(data, GCHandleType.Weak);
        Print.Line($"Weak    target null: {weak.Target is null}  (아직 살아있음)");
        weak.Free();

        // WeakTrackResurrection: 파이널라이저 큐에서도 추적
        GCHandle wtr = GCHandle.Alloc(data, GCHandleType.WeakTrackResurrection);
        Print.Line($"WeakTrackResurrection  IsAllocated: {wtr.IsAllocated}");
        wtr.Free();
    }

    // ── 2-4. Pinned Object Heap (POH) ────────────────────────────────────────
    static void ShowPinnedObjectHeap()
    {
        Print.Section("2-4. POH (Pinned Object Heap) — .NET 5+");

        // GC.AllocateArray<T>(length, pinned: true) → POH에 할당
        // GC.Collect에서 이동하지 않음 → 고정 비용 없이 네이티브 포인터 사용 가능
        byte[] pohSmall = GC.AllocateArray<byte>(256, pinned: true);
        pohSmall.AsSpan().Fill(0xAB);

        Print.Line($"POH 256B  Gen: Gen{GC.GetGeneration(pohSmall)}  [0]: 0x{pohSmall[0]:X2}");

        // 초기화 생략 버전 (비초기화 → 성능 우선)
        byte[] pohLarge = GC.AllocateUninitializedArray<byte>(LohThreshold, pinned: true);
        Print.Line($"POH {LohThreshold}B (비초기화)  Gen: Gen{GC.GetGeneration(pohLarge)}  Length={pohLarge.Length}");

        // Gen2 수집 후에도 POH 객체는 이동하지 않음
        IntPtr addrBefore = GetArrayDataAddress(pohSmall);
        GC.Collect(2, GCCollectionMode.Forced, blocking: true);
        IntPtr addrAfter = GetArrayDataAddress(pohSmall);
        Print.Line($"Gen2 수집 전후 주소 일치: {addrBefore == addrAfter}  (POH는 이동 안 함)");

        GC.KeepAlive(pohSmall); GC.KeepAlive(pohLarge);
    }

    static unsafe IntPtr GetArrayDataAddress(byte[] arr)
    {
        fixed (byte* p = arr) return (IntPtr)p;
    }
}
