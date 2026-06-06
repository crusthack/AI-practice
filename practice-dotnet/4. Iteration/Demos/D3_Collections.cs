using System.Collections.Immutable;

namespace Iteration.Demos;

static class D3_Collections
{
    public static void Run()
    {
        Print.Header("3. 컬렉션 (Collections)");

        ShowArrays();
        ShowList();
        ShowDictionary();
        ShowHashSet();
        ShowQueueStack();
        ShowLinkedList();
        ShowSorted();
        ShowPriorityQueue();
        ShowImmutable();
    }

    // ── 배열 ──────────────────────────────────────────────────────────────────
    static void ShowArrays()
    {
        Print.Section("3-1. Array — 1D·2D·가변(jagged) + Array 클래스");
        {
            // 1차원
            int[] a1 = [10, 30, 20, 50, 40];
            Array.Sort(a1);
            Console.Write("    Sort:  "); Print.Items(a1, "");
            Console.WriteLine($"    BinarySearch(30): index={Array.BinarySearch(a1, 30)}");

            int[] a2 = new int[5];
            Array.Fill(a2, 7);
            Console.Write("    Fill(7):  "); Print.Items(a2, "");

            // Reverse
            Array.Reverse(a1);
            Console.Write("    Reverse:  "); Print.Items(a1, "");

            // 2차원 배열
            int[,] matrix = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
            Console.Write("    2D [1,2]: "); Console.WriteLine(matrix[1, 2]);
            Console.WriteLine($"    2D 차원: {matrix.GetLength(0)}×{matrix.GetLength(1)}");

            // 가변 배열 (jagged array)
            int[][] jagged = new int[3][];
            jagged[0] = [1];
            jagged[1] = [1, 2];
            jagged[2] = [1, 2, 3];
            Console.WriteLine($"    jagged[2][2]={jagged[2][2]}  " +
                              $"각 행 길이: {string.Join(",", jagged.Select(r => r.Length))}");

            // Array.Copy / CopyTo / Clone
            int[] src = [1, 2, 3, 4, 5];
            int[] dst = new int[5];
            Array.Copy(src, 1, dst, 0, 3);  // src[1..3] → dst[0..2]
            Console.Write("    Copy(src,1,dst,0,3): "); Print.Items(dst, "");

            // Span 슬라이스 (복사 없음)
            var span = src.AsSpan()[1..4];
            Console.Write("    AsSpan()[1..4]: "); Print.Items(span.ToArray(), "");
        }
    }

    // ── List<T> ───────────────────────────────────────────────────────────────
    static void ShowList()
    {
        Print.Section("3-2. List<T> — 동적 배열 전체 API");
        {
            var list = new List<int> { 5, 3, 8, 1, 9, 2, 7, 4, 6 };

            // 추가/삽입/제거
            list.Add(10);
            list.Insert(0, 0);      // 맨 앞에 삽입
            list.Remove(5);         // 값으로 제거
            list.RemoveAt(0);       // 인덱스로 제거
            list.RemoveAll(x => x > 8);  // 조건으로 제거

            Console.Write("    편집 후: "); Print.Items(list, "");

            // 검색
            list.Sort();
            Console.Write("    Sort:    "); Print.Items(list, "");
            Console.WriteLine($"    Contains(7): {list.Contains(7)}");
            Console.WriteLine($"    IndexOf(7):  {list.IndexOf(7)}");
            Console.WriteLine($"    BinarySearch(7): {list.BinarySearch(7)}");
            Console.WriteLine($"    Find(>5):   {list.Find(x => x > 5)}");
            Console.WriteLine($"    FindLast(<5): {list.FindLast(x => x < 5)}");

            // 변환
            var doubled = list.ConvertAll(x => x * 2);
            Console.Write("    ConvertAll(×2): "); Print.Items(doubled, "");

            // 범위 조작
            var sub = list.GetRange(1, 3);
            Console.Write("    GetRange(1,3): "); Print.Items(sub, "");

            // TrueForAll / Exists
            Console.WriteLine($"    TrueForAll(>0): {list.TrueForAll(x => x > 0)}");
            Console.WriteLine($"    Exists(>7):     {list.Exists(x => x > 7)}");

            // Sort with comparer (내림차순)
            list.Sort((a, b) => b.CompareTo(a));
            Console.Write("    내림차순: "); Print.Items(list, "");
        }
    }

    // ── Dictionary<K,V> ───────────────────────────────────────────────────────
    static void ShowDictionary()
    {
        Print.Section("3-3. Dictionary<K,V> — 해시 맵 전체 API");
        {
            var dict = new Dictionary<string, int>
            {
                ["one"]   = 1,
                ["two"]   = 2,
                ["three"] = 3,
            };

            // 추가/갱신/제거
            dict.Add("four", 4);
            dict["two"] = 22;           // 갱신 (키 있으면 덮어씀)
            dict.Remove("three");

            Console.WriteLine($"    Count: {dict.Count}");
            Console.WriteLine($"    ContainsKey(\"one\"): {dict.ContainsKey("one")}");
            Console.WriteLine($"    ContainsValue(22): {dict.ContainsValue(22)}");

            // TryGetValue — 안전한 조회 (예외 없음)
            if (dict.TryGetValue("two", out int val))
                Console.WriteLine($"    TryGetValue(\"two\"): {val}");

            // 없는 키 조회 → KeyNotFoundException
            try { _ = dict["missing"]; }
            catch (KeyNotFoundException) { Console.WriteLine("    없는 키 → KeyNotFoundException"); }

            // GetValueOrDefault
            Console.WriteLine($"    GetValueOrDefault(\"missing\", -1): {dict.GetValueOrDefault("missing", -1)}");

            // Keys / Values 컬렉션
            Console.Write("    Keys:   "); Print.Items(dict.Keys, "");
            Console.Write("    Values: "); Print.Items(dict.Values, "");

            // foreach (KeyValuePair / 분해)
            foreach (var (k, v) in dict)
                Console.Write($"    {k}={v}  ");
            Console.WriteLine();

            // 중첩 컬렉션
            var groups = new Dictionary<string, List<string>>
            {
                ["과일"] = ["사과", "바나나"],
                ["채소"] = ["당근", "시금치"],
            };
            groups["과일"].Add("딸기");
            Console.WriteLine($"    그룹: 과일={string.Join(",", groups["과일"])}");
        }
    }

    // ── HashSet<T> ────────────────────────────────────────────────────────────
    static void ShowHashSet()
    {
        Print.Section("3-4. HashSet<T> — 집합 연산");
        {
            var a = new HashSet<int> { 1, 2, 3, 4, 5 };
            var b = new HashSet<int> { 4, 5, 6, 7, 8 };

            Console.WriteLine($"    A: {{{string.Join(",", a)}}}");
            Console.WriteLine($"    B: {{{string.Join(",", b)}}}");

            // 집합 연산 (원본 수정)
            var union  = new HashSet<int>(a); union.UnionWith(b);
            var inter  = new HashSet<int>(a); inter.IntersectWith(b);
            var diff   = new HashSet<int>(a); diff.ExceptWith(b);
            var symDif = new HashSet<int>(a); symDif.SymmetricExceptWith(b);

            Console.WriteLine($"    A ∪ B: {{{string.Join(",", union)}}}");
            Console.WriteLine($"    A ∩ B: {{{string.Join(",", inter)}}}");
            Console.WriteLine($"    A − B: {{{string.Join(",", diff)}}}");
            Console.WriteLine($"    A △ B: {{{string.Join(",", symDif)}}}");

            // 부분집합 / 상위집합 판별
            var sub = new HashSet<int> { 1, 2 };
            Console.WriteLine($"    {{1,2}} ⊆ A: {sub.IsSubsetOf(a)}");
            Console.WriteLine($"    A ⊇ {{1,2}}: {a.IsSupersetOf(sub)}");
            Console.WriteLine($"    A ∩ B = ∅: {a.Overlaps(b) is false} (겹침: {a.Overlaps(b)})");
            Console.WriteLine($"    A = A': {a.SetEquals(new HashSet<int>(a))}");

            // 중복 제거 용도
            var withDups = new List<string> { "a", "b", "a", "c", "b", "d" };
            var unique   = new HashSet<string>(withDups);
            Console.Write("    중복제거: "); Print.Items(unique, "");
        }
    }

    // ── Queue / Stack ─────────────────────────────────────────────────────────
    static void ShowQueueStack()
    {
        Print.Section("3-5. Queue<T> (FIFO) / Stack<T> (LIFO)");
        {
            // Queue
            var queue = new Queue<string>();
            queue.Enqueue("첫번째");
            queue.Enqueue("두번째");
            queue.Enqueue("세번째");
            Console.WriteLine($"    Peek: {queue.Peek()}  Count: {queue.Count}");
            while (queue.Count > 0)
                Console.Write($"    Dequeue→{queue.Dequeue()}  ");
            Console.WriteLine();

            // TryDequeue (안전)
            if (!queue.TryDequeue(out string? item))
                Console.WriteLine("    TryDequeue on empty: false");

            // Stack
            var stack = new Stack<int>();
            for (int i = 1; i <= 5; i++) stack.Push(i);
            Console.WriteLine($"    Peek: {stack.Peek()}  Count: {stack.Count}");
            Console.Write("    Pop: ");
            while (stack.TryPop(out int n)) Console.Write($"{n} ");
            Console.WriteLine();
        }
    }

    // ── LinkedList<T> ─────────────────────────────────────────────────────────
    static void ShowLinkedList()
    {
        Print.Section("3-6. LinkedList<T> — 양방향 연결 리스트");
        {
            var ll = new LinkedList<string>();
            var node2 = ll.AddFirst("B");
            var node1 = ll.AddBefore(node2, "A");
            var node3 = ll.AddAfter(node2, "C");
            ll.AddLast("D");

            Console.Write("    순방향: ");
            for (var n = ll.First; n is not null; n = n.Next)
                Console.Write($"{n.Value} ");
            Console.WriteLine();

            Console.Write("    역방향: ");
            for (var n = ll.Last; n is not null; n = n.Previous)
                Console.Write($"{n.Value} ");
            Console.WriteLine();

            // O(1) 삽입/삭제
            ll.Remove(node2);
            ll.AddAfter(node1, "X");
            Console.Write("    B 제거, A 뒤 X 삽입: ");
            Print.Items(ll, "");
        }
    }

    // ── SortedList / SortedDictionary ────────────────────────────────────────
    static void ShowSorted()
    {
        Print.Section("3-7. SortedList vs SortedDictionary — 정렬 유지 맵");
        {
            // SortedList: 내부가 병렬 배열 → 인덱스 접근 가능, 삽입 O(n)
            var sl = new SortedList<string, int> { ["banana"] = 2, ["apple"] = 1, ["cherry"] = 3 };
            Console.Write("    SortedList Keys: "); Print.Items(sl.Keys, "");
            Console.WriteLine($"    Index 접근 Keys[0]: {sl.Keys[0]}  Values[0]: {sl.Values[0]}");

            // SortedDictionary: 레드-블랙 트리 → 삽입 O(log n), 인덱스 접근 불가
            var sd = new SortedDictionary<string, int> { ["banana"] = 2, ["apple"] = 1, ["cherry"] = 3 };
            Console.Write("    SortedDictionary: "); Print.Items(sd.Keys, "");

            // 역순 Comparer
            var rev = new SortedDictionary<int, string>(Comparer<int>.Create((a, b) => b.CompareTo(a)));
            rev.Add(3, "C"); rev.Add(1, "A"); rev.Add(2, "B");
            Console.Write("    역순 SortedDictionary: ");
            foreach (var (k, v) in rev) Console.Write($"{k}={v} ");
            Console.WriteLine();
        }
    }

    // ── PriorityQueue<T, P> ───────────────────────────────────────────────────
    static void ShowPriorityQueue()
    {
        Print.Section("3-8. PriorityQueue<T, P> — 우선순위 큐 (.NET 6+)");
        {
            var pq = new PriorityQueue<string, int>();
            pq.Enqueue("낮은 우선순위",  10);
            pq.Enqueue("가장 높은 우선순위", 1);
            pq.Enqueue("중간 우선순위",   5);
            pq.Enqueue("높은 우선순위",   2);

            Console.Write("    우선순위 순서: ");
            while (pq.TryDequeue(out string? task, out int priority))
                Console.Write($"[{priority}]{task} ");
            Console.WriteLine();
        }
    }

    // ── Immutable Collections ─────────────────────────────────────────────────
    static void ShowImmutable()
    {
        Print.Section("3-9. Immutable Collections (System.Collections.Immutable)");
        {
            // ImmutableArray<T> — struct 기반, 배열 성능
            var ia = ImmutableArray.Create(1, 2, 3, 4, 5);
            var ia2 = ia.Add(6).RemoveAt(0);   // 새 인스턴스 반환
            Console.Write("    ImmutableArray: "); Print.Items(ia, "");
            Console.Write("    Add(6).RemoveAt(0): "); Print.Items(ia2, "");

            // ImmutableList<T> — tree 기반, 임의 삽입 효율적
            var il = ImmutableList.Create("A", "B", "C");
            var il2 = il.Insert(1, "X").Remove("C");
            Console.Write("    ImmutableList: "); Print.Items(il, "");
            Console.Write("    Insert(1,X).Remove(C): "); Print.Items(il2, "");

            // ImmutableDictionary<K,V>
            var id = ImmutableDictionary<string, int>.Empty
                .Add("x", 1)
                .Add("y", 2)
                .SetItem("x", 99);  // 갱신
            Console.WriteLine($"    ImmutableDictionary x={id["x"]}  y={id["y"]}");

            // ImmutableHashSet<T>
            var ihs = ImmutableHashSet.Create(1, 2, 3);
            var ihs2 = ihs.Add(4).Remove(1);
            Console.Write("    ImmutableHashSet: "); Print.Items(ihs, "");
            Console.Write("    Add(4).Remove(1): "); Print.Items(ihs2, "");

            // Builder 패턴: 여러 변경을 일괄 적용 (성능 최적화)
            var builder = ImmutableList.CreateBuilder<int>();
            for (int i = 0; i < 5; i++) builder.Add(i * 10);
            var builtList = builder.ToImmutable();
            Console.Write("    Builder ToImmutable: "); Print.Items(builtList, "");
        }
    }
}
