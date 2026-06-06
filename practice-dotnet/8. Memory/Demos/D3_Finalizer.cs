using System.Runtime.InteropServices;

namespace Memory.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 파이널라이저 & 소멸 메커니즘
//   ~T(), GC.SuppressFinalize, GC.ReRegisterForFinalize,
//   객체 부활(Resurrection), CriticalFinalizerObject, SafeHandle
// ────────────────────────────────────────────────────────────────────────────
static class D3_Finalizer
{
    public static void Run()
    {
        Print.Header("3. 파이널라이저 & 소멸 메커니즘");

        ShowBasicFinalizer();
        ShowSuppressFinalize();
        ShowResurrection();
        ShowSafeHandle();
    }

    // ── 3-1. 기본 파이널라이저 ───────────────────────────────────────────────
    static void ShowBasicFinalizer()
    {
        Print.Section("3-1. ~T() — GC가 수집 시 파이널라이저 큐에 등록 후 실행");

        CreateAndDiscard();

        GC.Collect(0, GCCollectionMode.Forced, blocking: true);
        GC.WaitForPendingFinalizers(); // 파이널라이저 스레드 완료 대기
        Print.Line("WaitForPendingFinalizers 완료");
        Print.Line("⚠ 파이널라이저 실행 순서는 보장되지 않음 — 절대 의존하지 말 것");
    }

    static void CreateAndDiscard()
    {
        _ = new FinalizableObj("Alpha");
        // 메서드 종료 → 로컬 참조 소멸 → GC 대상
    }

    // ── 3-2. GC.SuppressFinalize ─────────────────────────────────────────────
    static void ShowSuppressFinalize()
    {
        Print.Section("3-2. GC.SuppressFinalize — Dispose 시 파이널라이저 억제");

        var obj = new FinalizableObj("Beta");
        obj.DisposeProperly(); // 내부에서 SuppressFinalize 호출

        GC.Collect();
        GC.WaitForPendingFinalizers();
        Print.Line("→ 파이널라이저가 실행되지 않음 (SuppressFinalize로 큐 진입 차단)");
    }

    // ── 3-3. 객체 부활(Resurrection) ────────────────────────────────────────
    static void ShowResurrection()
    {
        Print.Section("3-3. 객체 부활 — 파이널라이저에서 강한 참조 재할당");

        FinalizableObj.Resurrected = null;
        {
            _ = new ResurrectableObj("Gamma");
        } // 범위 탈출

        GC.Collect(0, GCCollectionMode.Forced, blocking: true);
        GC.WaitForPendingFinalizers(); // ~ResurrectableObj() 실행 → 부활

        if (FinalizableObj.Resurrected is { } risen)
        {
            Print.Line($"부활한 객체: {risen.Name}");

            // ReRegisterForFinalize: 부활 후 파이널라이저를 다시 등록
            // (부활 객체는 파이널라이저 큐에서 빠져나왔으므로 재등록 필요)
            GC.ReRegisterForFinalize(risen);
            Print.Line("ReRegisterForFinalize → 다음 GC에서 파이널라이저 재실행");
        }

        FinalizableObj.Resurrected = null; // 강한 참조 해제 → 2차 수집
        GC.Collect(2, GCCollectionMode.Forced, blocking: true);
        GC.WaitForPendingFinalizers();
        Print.Line("2차 GC 완료 — 이번에는 부활 없음");
    }

    // ── 3-4. CriticalFinalizerObject & SafeHandle ────────────────────────────
    static void ShowSafeHandle()
    {
        Print.Section("3-4. CriticalFinalizerObject & SafeHandle");

        // SafeHandle: CriticalFinalizerObject의 서브클래스
        //   일반 파이널라이저보다 늦게 실행 → OS 핸들을 절대 누수시키지 않음
        //   using으로 명시적 해제 → ReleaseHandle() 호출

        using var sh = new DemoSafeHandle(0xDEAD);
        Print.Line($"DemoSafeHandle  IsInvalid: {sh.IsInvalid}  IsClosed: {sh.IsClosed}");
        // using 블록 종료 → ReleaseHandle() 호출
    }
}

// ── 파이널라이저 데모 클래스들 (public → Tests에서 참조) ─────────────────────

public class FinalizableObj
{
    public string Name { get; }
    public static FinalizableObj? Resurrected; // 부활 데모용 전역 참조

    public FinalizableObj(string name)
    {
        Name = name;
        Console.WriteLine($"      [new] {name}");
    }

    ~FinalizableObj()
        => Console.WriteLine($"      [~FinalizableObj] {Name}");

    public void DisposeProperly()
    {
        Console.WriteLine($"      [{Name}] SuppressFinalize 호출");
        GC.SuppressFinalize(this);
    }
}

public sealed class ResurrectableObj : FinalizableObj
{
    public ResurrectableObj(string name) : base(name) { }

    ~ResurrectableObj()
    {
        Console.WriteLine($"      [~ResurrectableObj] {Name} → 부활!");
        Resurrected = this; // 강한 참조 재할당 → GC에서 살아남음
    }
}

public sealed class DemoSafeHandle : SafeHandle
{
    public DemoSafeHandle(int fakeHandle) : base(IntPtr.Zero, ownsHandle: true)
        => SetHandle(new IntPtr(fakeHandle));

    public override bool IsInvalid => handle == IntPtr.Zero;

    protected override bool ReleaseHandle()
    {
        Console.WriteLine($"      [DemoSafeHandle.ReleaseHandle] 0x{handle:X}");
        return true;
    }
}
