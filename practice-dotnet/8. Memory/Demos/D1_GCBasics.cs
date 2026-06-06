using System.Runtime;

namespace Memory.Demos;

// ────────────────────────────────────────────────────────────────────────────
// GC 기초
//   세대(Gen0/1/2), 수집 모드, 메모리 정보, WeakReference, LatencyMode,
//   TryStartNoGCRegion, GCMemoryInfo
// ────────────────────────────────────────────────────────────────────────────
static class D1_GCBasics
{
    public static void Run()
    {
        Print.Header("1. GC 기초 — 세대·수집·약한 참조");

        ShowGenerations();
        ShowMemoryInfo();
        ShowWeakReference();
        ShowLatencyMode();
        ShowNoGCRegion();
    }

    // ── 1-1. 세대(Generation) ────────────────────────────────────────────────
    static void ShowGenerations()
    {
        Print.Section("1-1. GC 세대 (Gen0 → Gen1 → Gen2 승격)");

        // 새로 할당된 객체 → Gen0
        object obj = new();
        Print.Line($"새 객체:       Gen{GC.GetGeneration(obj)}");

        // Gen0 수집 → 생존하면 Gen1
        GC.Collect(0, GCCollectionMode.Forced, blocking: true);
        Print.Line($"Gen0 수집 후:  Gen{GC.GetGeneration(obj)}");

        // Gen1 수집 → Gen2
        GC.Collect(1, GCCollectionMode.Forced, blocking: true);
        Print.Line($"Gen1 수집 후:  Gen{GC.GetGeneration(obj)}");

        Print.Line($"MaxGeneration: {GC.MaxGeneration}");
        Print.Line($"수집 횟수 — Gen0:{GC.CollectionCount(0)}  Gen1:{GC.CollectionCount(1)}  Gen2:{GC.CollectionCount(2)}");

        GC.KeepAlive(obj);
    }

    // ── 1-2. 메모리 정보 ────────────────────────────────────────────────────
    static void ShowMemoryInfo()
    {
        Print.Section("1-2. GC.GetTotalMemory & GCMemoryInfo");

        long before = GC.GetTotalMemory(forceFullCollection: false);

        // 100KB 할당
        var buckets = new byte[100][];
        for (int i = 0; i < 100; i++) buckets[i] = new byte[1024];

        long after = GC.GetTotalMemory(forceFullCollection: false);
        Print.Line($"할당 전: {before / 1024}KB  할당 후: {after / 1024}KB  증가: {(after - before) / 1024}KB");

        GCMemoryInfo info = GC.GetGCMemoryInfo();
        Print.Line($"HeapSize:             {info.HeapSizeBytes / 1024}KB");
        Print.Line($"Fragmented:           {info.FragmentedBytes}B");
        Print.Line($"TotalAvailableMemory: {info.TotalAvailableMemoryBytes / 1024 / 1024}MB");
        Print.Line($"HighMemLoadThreshold: {info.HighMemoryLoadThresholdBytes / 1024 / 1024}MB");

        buckets = null!;
        long cleaned = GC.GetTotalMemory(forceFullCollection: true);
        Print.Line($"GC 후: {cleaned / 1024}KB");
    }

    // ── 1-3. WeakReference<T> ──────────────────────────────────────────────
    static void ShowWeakReference()
    {
        Print.Section("1-3. WeakReference<T> — GC를 막지 않는 참조");

        byte[] arr = new byte[10 * 1024]; // 10KB
        var weak = new WeakReference<byte[]>(arr);

        Print.Line($"강한 참조 유지 → alive: {weak.TryGetTarget(out _)}");

        arr = null!;
        GC.Collect(2, GCCollectionMode.Forced, blocking: true);
        GC.WaitForPendingFinalizers();

        Print.Line($"참조 해제 + GC 후 → alive: {weak.TryGetTarget(out _)}");

        // 캐시 패턴 — 메모리 압박 시 자동 해제
        string[] data = Enumerable.Range(0, 50).Select(i => i.ToString()).ToArray();
        var cache = new WeakReference<string[]>(data);
        Print.Line($"캐시 hit: {cache.TryGetTarget(out var hit)}  count={hit?.Length}");
    }

    // ── 1-4. GCSettings.LatencyMode ────────────────────────────────────────
    static void ShowLatencyMode()
    {
        Print.Section("1-4. GCSettings.LatencyMode — GC 영향 제어");

        var original = GCSettings.LatencyMode;
        Print.Line($"기본:         {original}");
        Print.Line($"IsServerGC:   {GCSettings.IsServerGC}");

        // LowLatency: Gen2 수집 억제 (저지연 구간)
        GCSettings.LatencyMode = GCLatencyMode.LowLatency;
        Print.Line($"LowLatency:   {GCSettings.LatencyMode}");

        GCSettings.LatencyMode = original;
        Print.Line($"복원:         {GCSettings.LatencyMode}");

        // SustainedLowLatency: LowLatency보다 장기 운영 가능 (단, 서버 GC에서만)
        Print.Line($"SustainedLowLatency = {(int)GCLatencyMode.SustainedLowLatency}  (서버 GC 전용)");
    }

    // ── 1-5. GC.TryStartNoGCRegion ──────────────────────────────────────────
    static void ShowNoGCRegion()
    {
        Print.Section("1-5. GC.TryStartNoGCRegion — GC 없는 임계 구간");

        const long budget = 4 * 1024 * 1024; // 4MB
        bool started = GC.TryStartNoGCRegion(budget, disallowFullBlockingGC: false);
        Print.Line($"NoGCRegion 시작: {started}  (budget {budget / 1024}KB)");

        if (started)
        {
            try
            {
                // 이 구간 내에서 GC가 발생하지 않음 (budget 초과 전까지)
                byte[] buf = new byte[64 * 1024];
                buf.AsSpan().Fill(0xFF);
                Print.Line($"GC 없는 구간에서 64KB 할당 및 채우기 완료  [0]=0x{buf[0]:X2}");
            }
            finally
            {
                GC.EndNoGCRegion();
                Print.Line("NoGCRegion 종료");
            }
        }
    }
}
