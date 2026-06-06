using System.Runtime.InteropServices;

namespace Memory.Demos;

// ────────────────────────────────────────────────────────────────────────────
// IDisposable & IAsyncDisposable
//   완전한 Dispose 패턴 (Dispose(bool) + SuppressFinalize + ~T),
//   상속 계층 Dispose, using 선언(C# 8+), IAsyncDisposable, SafeHandle 래퍼
// ────────────────────────────────────────────────────────────────────────────
static class D4_Dispose
{
    public static void Run()
    {
        Print.Header("4. IDisposable & IAsyncDisposable 패턴");

        ShowBasicDispose();
        ShowInheritedDispose();
        ShowUsingDeclaration();
        ShowAsyncDispose();
        ShowSafeHandleWrapper();
    }

    // ── 4-1. 완전한 Dispose 패턴 ─────────────────────────────────────────────
    static void ShowBasicDispose()
    {
        Print.Section("4-1. 완전한 Dispose 패턴 (Dispose(bool) + ~T)");

        ManagedResource res;
        using (res = new ManagedResource("R1"))
            res.Use();
        // using 블록 종료 → Dispose(true) → SuppressFinalize

        Print.Line($"using 후 IsDisposed: {res.IsDisposed}");

        try { res.Use(); }
        catch (ObjectDisposedException ex)
        { Print.Line($"ObjectDisposedException: {ex.Message[..40]}..."); }
    }

    // ── 4-2. 상속 계층 Dispose ───────────────────────────────────────────────
    static void ShowInheritedDispose()
    {
        Print.Section("4-2. 상속 계층에서의 Dispose — base.Dispose(disposing) 체인");

        using var child = new ChildResource("C1");
        child.Use();
        // Dispose → ChildResource.Dispose(true) → base.Dispose(true)
    }

    // ── 4-3. using 선언 (C# 8+) ──────────────────────────────────────────────
    static void ShowUsingDeclaration()
    {
        Print.Section("4-3. using 선언 — 범위 끝에서 자동 Dispose (역순)");

        using var r1 = new ManagedResource("D1");
        using var r2 = new ManagedResource("D2");
        using var r3 = new ManagedResource("D3");
        Print.Line("세 리소스 활성 중... 범위 종료 시 D3→D2→D1 순으로 해제됨");
    }

    // ── 4-4. IAsyncDisposable ─────────────────────────────────────────────────
    static void ShowAsyncDispose()
    {
        Print.Section("4-4. IAsyncDisposable — await using");

        RunAsync().GetAwaiter().GetResult();

        static async Task RunAsync()
        {
            await using var ar = new AsyncResource("AR1");
            await ar.DoWorkAsync();
            // 블록 종료 → await ar.DisposeAsync()
        }
    }

    // ── 4-5. SafeHandle 기반 네이티브 리소스 래퍼 ────────────────────────────
    static void ShowSafeHandleWrapper()
    {
        Print.Section("4-5. SafeHandle 서브클래스 — 네이티브 파일 핸들 시뮬레이션");

        using var file = new NativeFileHandle("mock_file.dat");
        Print.Line($"NativeFileHandle  IsInvalid: {file.IsInvalid}  Handle: 0x{file.DangerousGetHandle():X}");
        // using 종료 → ReleaseHandle() 보장 실행 (CriticalFinalizerObject 덕분)
    }
}

// ── 완전한 Dispose 패턴 ───────────────────────────────────────────────────────

public class ManagedResource : IDisposable
{
    private bool _disposed;
    public string  Name       { get; }
    public bool    IsDisposed => _disposed;

    public ManagedResource(string name)
    {
        Name = name;
        Console.WriteLine($"      [{name}] 생성");
    }

    public void Use()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        Console.WriteLine($"      [{Name}] 사용 중");
    }

    // ① public Dispose() — 사용자가 호출
    public void Dispose()
    {
        Dispose(disposing: true);
        GC.SuppressFinalize(this);
    }

    // ② protected virtual Dispose(bool) — 상속 계층의 재정의 진입점
    protected virtual void Dispose(bool disposing)
    {
        if (_disposed) return;
        if (disposing)
            Console.WriteLine($"      [{Name}] 관리 리소스 해제");
        // 비관리 리소스 해제는 disposing 여부 무관
        _disposed = true;
    }

    // ③ 파이널라이저 — SuppressFinalize 없을 때 폴백
    ~ManagedResource()
    {
        Dispose(disposing: false);
        Console.WriteLine($"      [{Name}] 파이널라이저 (SuppressFinalize 없었다면 실행)");
    }
}

// 파생 클래스 — base.Dispose(disposing) 호출 필수
public sealed class ChildResource : ManagedResource
{
    private bool _childDisposed;

    public ChildResource(string name) : base(name)
        => Console.WriteLine($"      [{name}] 자식 리소스 획득");

    protected override void Dispose(bool disposing)
    {
        if (_childDisposed) return;
        if (disposing)
            Console.WriteLine($"      [{Name}] 자식 리소스 해제");
        _childDisposed = true;
        base.Dispose(disposing); // ← 필수
    }
}

// IAsyncDisposable
public sealed class AsyncResource(string name) : IAsyncDisposable
{
    public async Task DoWorkAsync()
    {
        Console.WriteLine($"      [{name}] 비동기 작업 시작");
        await Task.Delay(10);
        Console.WriteLine($"      [{name}] 비동기 작업 완료");
    }

    public async ValueTask DisposeAsync()
    {
        Console.WriteLine($"      [{name}] DisposeAsync 시작");
        await Task.Delay(5);
        Console.WriteLine($"      [{name}] DisposeAsync 완료");
    }
}

// SafeHandle 서브클래스 — OS 핸들 안전 래퍼
public sealed class NativeFileHandle : SafeHandle
{
    public NativeFileHandle(string path) : base(IntPtr.Zero, ownsHandle: true)
    {
        // 실제: P/Invoke CreateFile 등 호출
        int fakeHandle = Math.Abs(path.GetHashCode()) % 0xFFFF + 1;
        SetHandle(new IntPtr(fakeHandle));
        Console.WriteLine($"      [{path}] 핸들 할당: 0x{fakeHandle:X}");
    }

    public override bool IsInvalid => handle == IntPtr.Zero;

    protected override bool ReleaseHandle()
    {
        Console.WriteLine($"      [NativeFileHandle.ReleaseHandle] 0x{handle:X}  해제 완료");
        return true;
    }
}
