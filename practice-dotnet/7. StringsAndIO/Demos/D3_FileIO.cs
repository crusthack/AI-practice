namespace StringsAndIO.Demos;

static class D3_FileIO
{
    private static readonly string TempDir =
        Path.Combine(Path.GetTempPath(), "DotNetPractice_StringsIO");

    public static void Run()
    {
        Directory.CreateDirectory(TempDir);
        try
        {
            Print.Header("3. 파일 · 디렉터리 · Path");
            ShowPathOperations();
            ShowFileOperations();
            ShowDirectoryOperations();
            ShowStreamIO();
            ShowAsyncFileIO().GetAwaiter().GetResult();
        }
        finally
        {
            Directory.Delete(TempDir, recursive: true);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowPathOperations()
    {
        Print.Section("3-1. Path — 경로 조작");

        string file = @"C:\Users\user\Documents\report.xlsx";

        Print.Line($"GetFileName:          {Path.GetFileName(file)}");
        Print.Line($"GetFileNameWithoutExt:{Path.GetFileNameWithoutExtension(file)}");
        Print.Line($"GetExtension:         {Path.GetExtension(file)}");
        Print.Line($"GetDirectoryName:     {Path.GetDirectoryName(file)}");
        Print.Line($"GetFullPath('..'):     {Path.GetFullPath("..")}");

        // Combine — OS 독립적 경로 구성
        string joined = Path.Combine("base", "sub", "file.txt");
        Print.Line($"Combine:              {joined}");

        // Path.Join — null 세그먼트 허용
        string joined2 = Path.Join(TempDir, "data", "output.csv");
        Print.Line($"Join:                 {joined2}");

        // 임시 경로
        Print.Line($"TempPath:             {Path.GetTempPath()}");
        Print.Line($"TempFileName:         {Path.GetTempFileName()}");
        Print.Line($"PathSeparator:        '{Path.PathSeparator}'  DirSeparator: '{Path.DirectorySeparatorChar}'");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowFileOperations()
    {
        Print.Section("3-2. File — 텍스트 · 바이너리 읽기쓰기");

        string txtPath = Path.Combine(TempDir, "sample.txt");
        string binPath = Path.Combine(TempDir, "sample.bin");

        // 텍스트 쓰기 (UTF-8)
        File.WriteAllText(txtPath, "첫 번째 줄\n두 번째 줄\n세 번째 줄");
        File.AppendAllText(txtPath, "\n네 번째 줄");

        // 텍스트 읽기
        string all  = File.ReadAllText(txtPath);
        string[] lines = File.ReadAllLines(txtPath);
        Print.Line($"전체 문자: {all.Length}  줄 수: {lines.Length}");
        foreach (var (line, i) in lines.Select((l, i) => (l, i)))
            Print.Line($"  [{i+1}] {line}");

        // 바이너리 쓰기/읽기
        byte[] bytes = [0x48, 0x65, 0x6C, 0x6C, 0x6F]; // "Hello"
        File.WriteAllBytes(binPath, bytes);
        byte[] read = File.ReadAllBytes(binPath);
        Print.Line($"바이너리: [{string.Join(", ", read.Select(b => $"0x{b:X2}"))}]");

        // File 정보
        var fi = new FileInfo(txtPath);
        Print.Line($"FileInfo: {fi.Name}  {fi.Length}바이트  {fi.LastWriteTime:HH:mm:ss}");

        // 복사, 이동, 삭제
        string copyPath = Path.Combine(TempDir, "copy.txt");
        File.Copy(txtPath, copyPath, overwrite: true);
        Print.Line($"복사됨: {File.Exists(copyPath)}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowDirectoryOperations()
    {
        Print.Section("3-3. Directory — 열거 · 생성 · 검색");

        string subDir = Path.Combine(TempDir, "sub");
        Directory.CreateDirectory(subDir);
        File.WriteAllText(Path.Combine(subDir, "a.txt"), "A");
        File.WriteAllText(Path.Combine(subDir, "b.csv"), "B");

        // 파일 열거 (패턴 포함)
        var txtFiles = Directory.GetFiles(TempDir, "*.txt", SearchOption.AllDirectories);
        Print.Line($"*.txt 파일: {txtFiles.Length}개");

        // EnumerateFiles — lazy (대용량 디렉터리에 유리)
        var csvFiles = Directory.EnumerateFiles(TempDir, "*.csv", SearchOption.AllDirectories);
        Print.Line($"*.csv:      {csvFiles.Count()}개");

        // DirectoryInfo
        var di = new DirectoryInfo(TempDir);
        Print.Line($"DirectoryInfo: {di.Name}  Full: ...{di.FullName[^Math.Min(30, di.FullName.Length)..]}");

        // Path.GetRelativePath
        string rel = Path.GetRelativePath(TempDir, Path.Combine(subDir, "a.txt"));
        Print.Line($"RelativePath:  {rel}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowStreamIO()
    {
        Print.Section("3-4. Stream · StreamReader · StreamWriter");

        string path = Path.Combine(TempDir, "stream.txt");

        // StreamWriter — 인코딩 지정
        using (var sw = new StreamWriter(path, append: false, System.Text.Encoding.UTF8))
        {
            sw.WriteLine("StreamWriter로 작성");
            sw.Write("버퍼된 출력: ");
            sw.WriteLine(DateTime.Now.ToString("HH:mm:ss"));
        }

        // StreamReader — 라인별 읽기
        using (var sr = new StreamReader(path, System.Text.Encoding.UTF8))
        {
            string? line;
            int lineNum = 0;
            while ((line = sr.ReadLine()) is not null)
                Print.Line($"  L{++lineNum}: {line}");
        }

        // BinaryWriter / BinaryReader
        string binPath = Path.Combine(TempDir, "data.bin");
        using (var bw = new BinaryWriter(File.Open(binPath, FileMode.Create)))
        {
            bw.Write(42);       // int
            bw.Write(3.14);     // double
            bw.Write("hello");  // string (길이 접두사 포함)
        }
        using (var br = new BinaryReader(File.Open(binPath, FileMode.Open)))
        {
            Print.Line($"  int={br.ReadInt32()}  double={br.ReadDouble():F2}  str={br.ReadString()}");
        }

        // MemoryStream — 메모리 내 스트림
        using var ms = new MemoryStream();
        using (var sw2 = new StreamWriter(ms, leaveOpen: true))
            sw2.Write("메모리 스트림");
        ms.Position = 0;
        using var sr2 = new StreamReader(ms);
        Print.Line($"  MemoryStream: {sr2.ReadToEnd()}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowAsyncFileIO()
    {
        Print.Section("3-5. 비동기 파일 I/O — ReadAllTextAsync · WriteAllTextAsync");

        string path = Path.Combine(TempDir, "async.txt");

        // 비동기 쓰기
        await File.WriteAllTextAsync(path,
            string.Join("\n", Enumerable.Range(1, 5).Select(i => $"줄 {i}")));

        // 비동기 읽기
        string content = await File.ReadAllTextAsync(path);
        Print.Line($"비동기 읽기 완료: {content.Split('\n').Length}줄");

        // 라인별 비동기 열거
        await foreach (var line in ReadLinesAsync(path))
            Console.Write($" [{line.TrimEnd()}]");
        Console.WriteLine();

        static async IAsyncEnumerable<string> ReadLinesAsync(string p)
        {
            using var sr = new StreamReader(p);
            string? line;
            while ((line = await sr.ReadLineAsync()) is not null)
                yield return line;
        }
    }
}
