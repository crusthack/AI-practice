using System.Buffers;
using System.Text;

namespace StringsAndIO.Demos;

static class D5_Encoding
{
    public static void Run()
    {
        Print.Header("5. 인코딩 · Base64 · 해싱 · Encoding");
        ShowEncodings();
        ShowBase64();
        ShowHashing();
        ShowArrayPool();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowEncodings()
    {
        Print.Section("5-1. Encoding — UTF-8 · UTF-16 · EUC-KR");

        string text = "Hello, 안녕하세요! 🌍";

        var encodings = new[]
        {
            Encoding.UTF8,
            Encoding.Unicode,    // UTF-16 LE
            Encoding.ASCII,
        };

        foreach (var enc in encodings)
        {
            byte[] bytes = enc.GetBytes(text);
            string back  = enc.GetString(bytes);
            Print.Line($"  {enc.EncodingName,-20}: {bytes.Length,3} bytes  round-trip: {back == text}");
        }

        // UTF-8 수동 인코딩 (할당 없이)
        byte[] buf = new byte[Encoding.UTF8.GetMaxByteCount(text.Length)];
        int written = Encoding.UTF8.GetBytes(text, buf);
        Print.Line($"  UTF-8 GetBytes(span): {written}바이트");

        // BOM
        byte[] withBom    = Encoding.UTF8.GetPreamble();
        byte[] withoutBom = Encoding.UTF8.GetBytes("A");
        Print.Line($"  UTF-8 BOM: [{string.Join(" ", withBom.Select(b => $"0x{b:X2}"))}]  (없으면 빈 배열)");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowBase64()
    {
        Print.Section("5-2. Base64 · Base64Url");

        byte[] data = "안녕하세요 Hello 🌍"u8.ToArray();

        // 표준 Base64
        string b64 = Convert.ToBase64String(data);
        byte[] back = Convert.FromBase64String(b64);
        Print.Line($"  Base64:    {b64}");
        Print.Line($"  복원:      {Encoding.UTF8.GetString(back)}");

        // Base64Url (URL-safe, 패딩 없음)
        string url64 = Convert.ToBase64String(data)
            .Replace('+', '-').Replace('/', '_').TrimEnd('=');
        Print.Line($"  Base64Url: {url64}");

        // Span 기반 변환 (할당 최소화)
        Span<byte> src = stackalloc byte[] { 1, 2, 3, 4, 5 };
        Span<char> b64Buf = stackalloc char[8];
        Convert.TryToBase64Chars(src, b64Buf, out int charsWritten);
        Print.Line($"  Span Base64: {new string(b64Buf[..charsWritten])}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowHashing()
    {
        Print.Section("5-3. 해시 — SHA-256 · HMAC · 빠른 해시");

        string password = "Secret@Password123";
        byte[] data     = Encoding.UTF8.GetBytes(password);

        // SHA-256
        using var sha256 = System.Security.Cryptography.SHA256.Create();
        byte[] hash = sha256.ComputeHash(data);
        string hexHash = Convert.ToHexString(hash);
        Print.Line($"  SHA-256: {hexHash[..32]}...");

        // SHA-256 한 번에 (정적 메서드)
        byte[] hash2 = System.Security.Cryptography.SHA256.HashData(data);
        Print.Line($"  동일:    {hexHash == Convert.ToHexString(hash2)}");

        // HMAC-SHA256 (메시지 인증)
        byte[] key  = new byte[32];
        System.Security.Cryptography.RandomNumberGenerator.Fill(key);
        using var hmac = new System.Security.Cryptography.HMACSHA256(key);
        byte[] mac = hmac.ComputeHash(data);
        Print.Line($"  HMAC-SHA256: {Convert.ToHexString(mac)[..32]}...");

        // HashCode (구조적 동등성, 보안 목적 아님)
        var hc = new HashCode();
        hc.Add("field1");
        hc.Add(42);
        hc.Add(3.14);
        Print.Line($"  HashCode.Combine: {HashCode.Combine("a", 1, 2.0)}");
        Print.Line($"  HashCode.Add:     {hc.ToHashCode()}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowArrayPool()
    {
        Print.Section("5-4. ArrayPool<T> · MemoryPool<T> — 재사용 버퍼");

        // ArrayPool: 임시 버퍼가 필요할 때 GC 압력 줄이기
        var pool = ArrayPool<byte>.Shared;

        byte[]? buffer = null;
        try
        {
            buffer = pool.Rent(4096);     // 최소 4096바이트 보장 (더 클 수도 있음)
            Print.Line($"  Rent(4096) 실제 크기: {buffer.Length}");

            // 버퍼 사용
            "Hello from pool"u8.CopyTo(buffer);
            Print.Line($"  데이터: {Encoding.UTF8.GetString(buffer, 0, 15)}");
        }
        finally
        {
            if (buffer is not null)
                pool.Return(buffer, clearArray: true); // 반환 시 내용 초기화
        }

        // MemoryPool: Memory<T> 기반 (Slice/Span 지원)
        using var memPool = MemoryPool<byte>.Shared.Rent(1024);
        var mem = memPool.Memory[..16];
        "MemoryPool OK!"u8.CopyTo(mem.Span);
        Print.Line($"  MemoryPool: {Encoding.UTF8.GetString(mem.Span)}");
    }
}
