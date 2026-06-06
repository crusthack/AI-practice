namespace Iteration.Demos;

static class D1_Loops
{
    public static void Run()
    {
        Print.Header("1. 반복문 (Loops)");

        // ── 1-1. for — 모든 변형 ──────────────────────────────────────────────
        Print.Section("1-1. for 루프 변형");
        {
            // 기본
            var buf = new List<int>();
            for (int i = 0; i < 5; i++) buf.Add(i);
            Console.Write("    기본:   "); Print.Items(buf, "");

            // 역방향
            buf.Clear();
            for (int i = 4; i >= 0; i--) buf.Add(i);
            Console.Write("    역방향: "); Print.Items(buf, "");

            // 다중 변수
            buf.Clear();
            for (int i = 0, j = 10; i < j; i++, j--) buf.Add(i * j);
            Console.Write("    다중변수(i*j): "); Print.Items(buf, "");

            // 무한 루프 + break
            int count = 0;
            for (;;) { if (++count >= 5) break; }
            Console.WriteLine($"    for(;;)+break: count={count}");

            // 스텝 2
            buf.Clear();
            for (int i = 0; i <= 10; i += 2) buf.Add(i);
            Console.Write("    짝수:   "); Print.Items(buf, "");
        }

        // ── 1-2. while / do-while ────────────────────────────────────────────
        Print.Section("1-2. while / do-while");
        {
            // while: 조건 먼저 검사 → 0번 실행 가능
            int n = 1, product = 1;
            while (n <= 5) product *= n++;
            Console.WriteLine($"    while: 5! = {product}");

            // do-while: 본문 먼저 실행 → 최소 1번
            int input = 0;
            do { input++; } while (input < 3);
            Console.WriteLine($"    do-while: 최소 1회 실행, 결과={input}");

            // while + continue/break
            int sum = 0;
            n = 0;
            while (n < 20)
            {
                n++;
                if (n % 2 == 0) continue;  // 짝수 건너뜀
                if (n > 10) break;          // 10 초과 시 종료
                sum += n;
            }
            Console.WriteLine($"    10 이하 홀수 합: {sum}");
        }

        // ── 1-3. foreach ─────────────────────────────────────────────────────
        Print.Section("1-3. foreach — 컬렉션·배열·문자열·range");
        {
            // 배열
            int[] arr = [10, 20, 30, 40, 50];
            int total = 0;
            foreach (int x in arr) total += x;
            Console.WriteLine($"    배열 합계: {total}");

            // 문자열 (IEnumerable<char>)
            string s = "Hello";
            var uppers = new List<char>();
            foreach (char c in s) uppers.Add(char.ToUpper(c));
            Console.WriteLine($"    문자열 순회: {new string([.. uppers])}");

            // Dictionary
            var dict = new Dictionary<string, int> { ["A"] = 1, ["B"] = 2, ["C"] = 3 };
            foreach (var (key, val) in dict)
                Console.Write($"    {key}={val}  ");
            Console.WriteLine();

            // foreach + index via Select
            string[] names = ["Alice", "Bob", "Carol"];
            foreach (var (name, idx) in names.Select((x, i) => (x, i)))
                Console.Write($"    [{idx}]{name}  ");
            Console.WriteLine();
        }

        // ── 1-4. ref foreach — Span/배열 요소 직접 수정 ──────────────────────
        Print.Section("1-4. ref foreach — Span<T> 요소 in-place 수정");
        {
            int[] data = [1, 2, 3, 4, 5];
            Span<int> span = data;

            foreach (ref int x in span) x *= x;  // 제곱 in-place

            Console.Write("    제곱: "); Print.Items(data, "");
        }

        // ── 1-5. Index(^) & Range(..) 연산자 ─────────────────────────────────
        Print.Section("1-5. Index(^) & Range(..) 연산자 (C# 8+)");
        {
            int[] a = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

            Console.WriteLine($"    a[^1]    = {a[^1]}   (마지막)");
            Console.WriteLine($"    a[^2]    = {a[^2]}   (끝에서 2번째)");
            Console.Write("    a[2..5]  = "); Print.Items(a[2..5], "");
            Console.Write("    a[^3..]  = "); Print.Items(a[^3..], "");
            Console.Write("    a[..3]   = "); Print.Items(a[..3], "");
            Console.Write("    a[..]    = "); Print.Items(a[..], "");

            // Range를 변수에 저장
            Range r = 1..^1;
            Console.Write($"    a[{r}]  = "); Print.Items(a[r], "");

            // Index를 변수에 저장
            Index last = ^1;
            Console.WriteLine($"    a[last]  = {a[last]}");
        }

        // ── 1-6. break / continue / goto (중첩 루프 탈출) ────────────────────
        Print.Section("1-6. break / continue / goto 레이블");
        {
            // break: 현재 루프만 탈출
            var found = new List<(int, int)>();
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                {
                    if (j == 2) break;   // 안쪽 루프만 탈출
                    found.Add((i, j));
                }
            Console.WriteLine($"    break: {found.Count}쌍 (j<2만)");

            // continue: 현재 반복 건너뜀
            int skipSum = 0;
            for (int i = 1; i <= 10; i++)
            {
                if (i % 3 == 0) continue;
                skipSum += i;
            }
            Console.WriteLine($"    continue: 3의 배수 제외 합={skipSum}");

            // goto: 외부 레이블로 점프 (중첩 루프 전체 탈출)
            bool escaped = false;
            for (int i = 0; i < 5; i++)
                for (int j = 0; j < 5; j++)
                    if (i == 2 && j == 3) { escaped = true; goto afterNestedLoop; }

        afterNestedLoop:
            Console.WriteLine($"    goto 탈출: escaped={escaped}");
        }

        // ── 1-7. Parallel.For / ForEach ───────────────────────────────────────
        Print.Section("1-7. Parallel.For / ForEach — 병렬 루프");
        {
            int parallelSum = 0;
            Parallel.For(1, 101, i => Interlocked.Add(ref parallelSum, i));
            Console.WriteLine($"    Parallel.For 1~100 합: {parallelSum}");

            var squares = new int[5];
            Parallel.ForEach(Enumerable.Range(0, 5), i => squares[i] = i * i);
            Console.Write("    Parallel.ForEach 제곱: "); Print.Items(squares, "");
        }
    }
}
