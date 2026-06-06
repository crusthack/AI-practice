using System.Buffers;
using System.Runtime.InteropServices;
using System.Text;

namespace Containers.Demos;

// ────────────────────────────────────────────────────────────────────────────
// Span<T> / Memory<T> / ArrayPool<T>
//   Span<T>         — 스택 전용 ref struct. 복사 없이 연속 메모리 슬라이스
//   ReadOnlySpan<T> — 읽기 전용 Span (string → char 슬라이스 포함)
//   Memory<T>       — 힙 저장 가능, 비동기 메서드에서도 사용
//   IMemoryOwner<T> — 수명 관리 포함 Memory (MemoryPool에서 렌트)
//   ArrayPool<T>    — 배열 재사용 풀 (heap 할당 최소화)
// ────────────────────────────────────────────────────────────────────────────
static class D1_SpanMemory
{
    public static void Run()
    {
        Print.Header("1. Span<T> / Memory<T> / ArrayPool<T>");

        ShowSpanBasics();
        ShowReadOnlySpanString();
        ShowMemory();
        ShowArrayPool();
        ShowMemoryMarshal();
        ShowStackalloc();
    }

    // ── 1-1. Span<T> 기초 ────────────────────────────────────────────────────
    static void ShowSpanBasics()
    {
        Print.Section("1-1. Span<T> — 복사 없는 슬라이스");

        int[] array = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100];
        Span<int> span = array;

        // 슬라이스 — 새 배열 할당 없음
        Span<int> mid = span[2..7];
        Print.Items(mid.ToArray(), "span[2..7]: ");

        // 수정 → 원본 배열에 반영
        mid[0] = 999;
        Print.Line($"mid[0]=999 → array[2]={array[2]}  (원본 공유)");

        // Span.Fill / Span.CopyTo / Span.Clear
        Span<int> buf = new int[5];
        buf.Fill(7);
        Print.Items(buf.ToArray(), "Fill(7): ");

        span[..5].CopyTo(buf);
        Print.Items(buf.ToArray(), "CopyTo: ");

        // 역방향 검색 (LastIndexOf)
        Span<int> data = [1, 2, 3, 2, 1];
        Print.Line($"IndexOf(2):     {data.IndexOf(2)}");
        Print.Line($"LastIndexOf(2): {data.LastIndexOf(2)}");

        // ref foreach — 요소 직접 수정
        Span<int> sq = [1, 2, 3, 4, 5];
        foreach (ref int x in sq) x *= x;
        Print.Items(sq.ToArray(), "ref foreach ²: ");
    }

    // ── 1-2. ReadOnlySpan<char> — 무할당 문자열 처리 ─────────────────────
    static void ShowReadOnlySpanString()
    {
        Print.Section("1-2. ReadOnlySpan<char> — 문자열 무할당 파싱");

        // string → ReadOnlySpan<char> 변환 (복사 없음)
        string csv = "Alice,30,Seoul,CS";
        ReadOnlySpan<char> span = csv.AsSpan();

        // Span 기반 분할 (allocation free)
        Print.Line("CSV 파싱 (무할당):");
        int start = 0;
        for (int i = 0; i <= span.Length; i++)
        {
            if (i == span.Length || span[i] == ',')
            {
                Console.Write($"      [{span[start..i]}]  ");
                start = i + 1;
            }
        }
        Console.WriteLine();

        // 정수 파싱 — int.TryParse(ReadOnlySpan<char>)
        ReadOnlySpan<char> numSpan = "   42   ".AsSpan().Trim();
        int.TryParse(numSpan, out int parsed);
        Print.Line($"TryParse(\"   42   \".Trim()): {parsed}");

        // StartsWith / EndsWith / Contains on Span
        ReadOnlySpan<char> text = "Hello, World!".AsSpan();
        Print.Line($"StartsWith(Hello): {text.StartsWith("Hello")}");
        Print.Line($"EndsWith(!):       {text.EndsWith("!")}");
        Print.Line($"IndexOf(W):        {text.IndexOf('W')}");

        // Slice & Compare
        var word = text[7..12];
        Print.Line($"Slice[7..12]: {word}  SequenceEqual(World): {word.SequenceEqual("World")}");
    }

    // ── 1-3. Memory<T> — 힙 보관 가능 슬라이스 ──────────────────────────
    static void ShowMemory()
    {
        Print.Section("1-3. Memory<T> — 힙 저장·비동기 사용 가능");

        int[] arr = Enumerable.Range(1, 10).ToArray();
        Memory<int> mem = arr;

        // Span으로 변환해서 작업
        Span<int> s = mem.Span;
        s[0] = 99;
        Print.Line($"Memory.Span[0]=99 → arr[0]={arr[0]}");

        // 슬라이스
        Memory<int> slice = mem[2..5];
        Print.Items(slice.ToArray(), "mem[2..5]: ");

        // ReadOnlyMemory<T>
        ReadOnlyMemory<char> roMem = "Hello World".AsMemory();
        Print.Line($"ReadOnlyMemory: {roMem}  Length={roMem.Length}");
        Print.Line($"Slice[6..]: {roMem[6..]}");

        // ProcessAsync 시뮬레이션 (Memory는 async에서 사용 가능)
        var task = ProcessAsync(mem[1..4]);
        task.Wait();
    }

    static async Task ProcessAsync(Memory<int> mem)
    {
        await Task.Delay(1);
        int sum = 0;
        foreach (int x in mem.Span) sum += x;
        Print.Line($"async ProcessAsync sum={sum}");
    }

    // ── 1-4. ArrayPool<T> — 배열 재사용 풀 ─────────────────────────────
    static void ShowArrayPool()
    {
        Print.Section("1-4. ArrayPool<T> — 배열 할당 최소화");

        // Shared 풀에서 렌트 (최소 요청 크기 보장, 실제 크기는 더 클 수 있음)
        int[] rented = ArrayPool<int>.Shared.Rent(100);
        Print.Line($"Rent(100) 실제 크기: {rented.Length}  (풀 구현에 따라 더 클 수 있음)");

        // 사용 — rented.Length 아닌 요청 크기(100)만 사용해야 함
        Span<int> span = rented.AsSpan(0, 100);
        span.Fill(42);
        Print.Line($"span[0..3]: {rented[0]},{rented[1]},{rented[2]},...");

        // 반드시 Return (clearArray: true → 민감 데이터 초기화)
        ArrayPool<int>.Shared.Return(rented, clearArray: false);
        Print.Line("Return 완료 (clearArray: false)");

        // Custom ArrayPool (최대 크기·버킷 수 제어)
        var customPool = ArrayPool<byte>.Create(maxArrayLength: 1024 * 64, maxArraysPerBucket: 10);
        byte[] buf = customPool.Rent(512);
        customPool.Return(buf);
        Print.Line($"Custom pool Rent(512) → {buf.Length}B  반환 완료");

        // using 패턴으로 안전한 렌트 (IMemoryOwner)
        using IMemoryOwner<byte> owner = MemoryPool<byte>.Shared.Rent(256);
        Memory<byte> rentedMem = owner.Memory;
        rentedMem.Span.Fill(0xFF);
        Print.Line($"MemoryPool Rent(256) → {rentedMem.Length}B  [0]={rentedMem.Span[0]:X2}");
        // Dispose() 시 자동 Return
    }

    // ── 1-5. MemoryMarshal — 타입 재해석 ────────────────────────────────
    static void ShowMemoryMarshal()
    {
        Print.Section("1-5. MemoryMarshal — 저수준 메모리 재해석");

        // Cast: byte[] → int[] (복사 없이 타입 재해석)
        byte[] bytes = [0x01, 0x00, 0x00, 0x00,  // int 1
                        0x02, 0x00, 0x00, 0x00];  // int 2
        ReadOnlySpan<int> ints = MemoryMarshal.Cast<byte, int>(bytes);
        Print.Line($"Cast byte[8] → int[2]: [{ints[0]}, {ints[1]}]");

        // GetReference: Span의 첫 번째 요소 ref 참조
        int[] arr = [10, 20, 30];
        ref int first = ref MemoryMarshal.GetReference(arr.AsSpan());
        first = 99;
        Print.Line($"GetReference 수정 → arr[0]={arr[0]}");

        // CreateReadOnlySpan: 단일 값을 Span으로 (복사 없음)
        int value = 42;
        ReadOnlySpan<int> single = MemoryMarshal.CreateReadOnlySpan(ref value, 1);
        Print.Line($"CreateReadOnlySpan: {single[0]}");

        // AsBytes: struct → byte 표현
        int[] intArr = [0x12345678];
        ReadOnlySpan<byte> asBytes = MemoryMarshal.AsBytes(intArr.AsSpan());
        Print.Items(asBytes.ToArray().Select(b => $"{b:X2}"), "AsBytes: ");
    }

    // ── 1-6. stackalloc + Span ───────────────────────────────────────────
    static void ShowStackalloc()
    {
        Print.Section("1-6. stackalloc + Span<T> — 스택 할당 버퍼");

        // stackalloc: 스택에 배열 할당 (unsafe 불필요, C# 7.2+)
        Span<int> buf = stackalloc int[8];
        for (int i = 0; i < buf.Length; i++) buf[i] = i * i;
        Print.Items(buf.ToArray(), "stackalloc int[8]: ");

        // 크기 조건부 stackalloc (큰 경우 힙으로 폴백)
        const int threshold = 256;
        int size = 10;
        Span<byte> data = size <= threshold
            ? stackalloc byte[size]
            : new byte[size];
        data.Fill(0xAB);
        Print.Line($"조건부 stackalloc[{size}]: [{data[0]:X2},...,{data[^1]:X2}]  IsStack={size <= threshold}");

        // StringBuilder 없이 ReadOnlySpan<char>로 문자열 조작
        Span<char> chars = stackalloc char[20];
        "Hello".AsSpan().CopyTo(chars);
        " World".AsSpan().CopyTo(chars[5..]);
        Print.Line($"stackalloc char: {new string(chars[..11])}");
    }
}
