namespace Containers.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 트리 & 그래프 (Tree & Graph)
//   BinarySearchTree<T> — BST: insert, search, 순회(3가지), 높이, 최솟값
//   Trie                — 접두사 트리: insert, search, startsWith, autocomplete
//   Graph<T>            — 인접 리스트: BFS, DFS, 위상 정렬
// ────────────────────────────────────────────────────────────────────────────
static class D3_TreeGraph
{
    public static void Run()
    {
        Print.Header("3. 트리 & 그래프 (Tree & Graph)");

        ShowBST();
        ShowTrie();
        ShowGraph();
    }

    // ── 3-1. Binary Search Tree ─────────────────────────────────────────────
    static void ShowBST()
    {
        Print.Section("3-1. BinarySearchTree<T> — 이진 탐색 트리");

        var bst = new BinarySearchTree<int>();
        int[] vals = [5, 3, 8, 1, 4, 7, 9, 2, 6];
        foreach (int v in vals) bst.Insert(v);
        Print.Line($"Insert {string.Join(",", vals)} → Count={bst.Count}  Height={bst.Height}");

        Print.Items(bst.InOrder(),   "InOrder  (정렬):    ");
        Print.Items(bst.PreOrder(),  "PreOrder (전위):    ");
        Print.Items(bst.PostOrder(), "PostOrder(후위):    ");
        Print.Items(bst.LevelOrder(),"LevelOrder(너비우선): ");

        Print.Line($"Contains(7): {bst.Contains(7)}  Contains(10): {bst.Contains(10)}");
        Print.Line($"Min={bst.Min}  Max={bst.Max}");

        // 삭제 (리프, 단일 자식, 두 자식 모두)
        bst.Remove(1);  // 리프
        bst.Remove(3);  // 단일 자식 (왼쪽: 2, 오른쪽: 4)
        bst.Remove(8);  // 두 자식 (7, 9)
        Print.Items(bst.InOrder(), "Remove(1,3,8) 후: ");
    }

    // ── 3-2. Trie (접두사 트리) ───────────────────────────────────────────
    static void ShowTrie()
    {
        Print.Section("3-2. Trie — 접두사 트리 (자동완성)");

        var trie = new Trie();
        string[] words = ["apple", "app", "application", "apply", "apt", "banana", "band", "bandwidth"];
        foreach (var w in words) trie.Insert(w);
        Print.Line($"Insert {words.Length}개 단어");

        Print.Line($"Search(\"app\"):         {trie.Search("app")}");
        Print.Line($"Search(\"application\"): {trie.Search("application")}");
        Print.Line($"Search(\"applet\"):      {trie.Search("applet")}");
        Print.Line($"StartsWith(\"app\"):     {trie.StartsWith("app")}");
        Print.Line($"StartsWith(\"xyz\"):     {trie.StartsWith("xyz")}");

        Print.Items(trie.GetWordsWithPrefix("app"), "autocomplete(\"app\"): ");
        Print.Items(trie.GetWordsWithPrefix("ban"), "autocomplete(\"ban\"): ");

        trie.Delete("app");
        Print.Line($"Delete(\"app\") → Search(\"app\")={trie.Search("app")}  StartsWith(\"app\")={trie.StartsWith("app")}");
    }

    // ── 3-3. Graph ─────────────────────────────────────────────────────────
    static void ShowGraph()
    {
        Print.Section("3-3. Graph<T> — 인접 리스트 (BFS·DFS·위상정렬)");

        // 무방향 그래프 (BFS/DFS)
        var g = new Graph<string>(directed: false);
        g.AddEdge("A", "B"); g.AddEdge("A", "C");
        g.AddEdge("B", "D"); g.AddEdge("B", "E");
        g.AddEdge("C", "F"); g.AddEdge("D", "G");

        Print.Items(g.BFS("A"), "BFS(A): ");
        Print.Items(g.DFS("A"), "DFS(A): ");

        // 연결 요소 탐색
        var g2 = new Graph<int>(directed: false);
        g2.AddEdge(1, 2); g2.AddEdge(2, 3);
        g2.AddEdge(4, 5);   // 별도 연결 요소
        g2.AddVertex(6);     // 독립 노드
        var components = g2.ConnectedComponents();
        Print.Line($"연결 요소 {components.Count}개:");
        foreach (var comp in components)
            Console.WriteLine($"      {{{string.Join(",", comp)}}}");

        // 방향 그래프 (DAG) — 위상 정렬
        var dag = new Graph<string>(directed: true);
        dag.AddEdge("세탁", "건조");
        dag.AddEdge("건조", "다림질");
        dag.AddEdge("세탁", "분리수거");
        dag.AddEdge("장보기", "요리");
        dag.AddEdge("요리", "식사");
        dag.AddEdge("다림질", "옷장정리");

        var topo = dag.TopologicalSort();
        Print.Items(topo, "위상 정렬: ");

        // 가중치 그래프 — 다익스트라
        var weighted = new WeightedGraph<string>();
        weighted.AddEdge("서울", "수원", 30);
        weighted.AddEdge("서울", "인천", 40);
        weighted.AddEdge("수원", "대전", 90);
        weighted.AddEdge("인천", "대전", 120);
        weighted.AddEdge("대전", "부산", 150);
        weighted.AddEdge("수원", "부산", 250);

        var (dist, path) = weighted.Dijkstra("서울", "부산");
        Print.Line($"Dijkstra 서울→부산: {dist}km  경로={string.Join("→", path)}");
    }
}

// ── BinarySearchTree<T> ────────────────────────────────────────────────────

public sealed class BinarySearchTree<T> where T : IComparable<T>
{
    private sealed class Node(T value)
    {
        public T     Value { get; set; } = value;
        public Node? Left, Right;
    }

    private Node? _root;
    public int Count  { get; private set; }
    public int Height => GetHeight(_root);

    public void Insert(T value)
    {
        _root = InsertRec(_root, value);
        Count++;
    }

    private static Node InsertRec(Node? node, T v)
    {
        if (node is null) return new Node(v);
        int cmp = v.CompareTo(node.Value);
        if (cmp < 0) node.Left  = InsertRec(node.Left,  v);
        else if (cmp > 0) node.Right = InsertRec(node.Right, v);
        return node;
    }

    public bool Contains(T value)
    {
        Node? cur = _root;
        while (cur is not null)
        {
            int cmp = value.CompareTo(cur.Value);
            if (cmp == 0) return true;
            cur = cmp < 0 ? cur.Left : cur.Right;
        }
        return false;
    }

    public void Remove(T value) { _root = RemoveRec(_root, value); Count--; }

    private static Node? RemoveRec(Node? node, T v)
    {
        if (node is null) return null;
        int cmp = v.CompareTo(node.Value);
        if (cmp < 0) { node.Left  = RemoveRec(node.Left,  v); return node; }
        if (cmp > 0) { node.Right = RemoveRec(node.Right, v); return node; }

        // 삭제 대상 발견
        if (node.Left  is null) return node.Right;
        if (node.Right is null) return node.Left;

        // 두 자식: 오른쪽 서브트리 최솟값으로 교체
        Node minRight = node.Right;
        while (minRight.Left is not null) minRight = minRight.Left;
        node.Value = minRight.Value;
        node.Right = RemoveRec(node.Right, minRight.Value);
        return node;
    }

    public T Min { get { var n = _root; while (n!.Left is not null) n = n.Left; return n.Value; } }
    public T Max { get { var n = _root; while (n!.Right is not null) n = n.Right; return n.Value; } }

    public IEnumerable<T> InOrder()    { var r = new List<T>(); InRec(_root, r);   return r; }
    public IEnumerable<T> PreOrder()   { var r = new List<T>(); PreRec(_root, r);  return r; }
    public IEnumerable<T> PostOrder()  { var r = new List<T>(); PostRec(_root, r); return r; }

    private static void InRec(Node? n, List<T> r)   { if (n is null) return; InRec(n.Left, r);   r.Add(n.Value); InRec(n.Right, r); }
    private static void PreRec(Node? n, List<T> r)  { if (n is null) return; r.Add(n.Value); PreRec(n.Left, r);  PreRec(n.Right, r); }
    private static void PostRec(Node? n, List<T> r) { if (n is null) return; PostRec(n.Left, r); PostRec(n.Right, r); r.Add(n.Value); }

    public IEnumerable<T> LevelOrder()
    {
        if (_root is null) return [];
        var result = new List<T>();
        var queue  = new Queue<Node>([_root]);
        while (queue.Count > 0)
        {
            var node = queue.Dequeue();
            result.Add(node.Value);
            if (node.Left  is not null) queue.Enqueue(node.Left);
            if (node.Right is not null) queue.Enqueue(node.Right);
        }
        return result;
    }

    private static int GetHeight(Node? n) =>
        n is null ? 0 : 1 + Math.Max(GetHeight(n.Left), GetHeight(n.Right));
}

// ── Trie ───────────────────────────────────────────────────────────────────

public sealed class Trie
{
    private sealed class Node
    {
        public Dictionary<char, Node> Children { get; } = new();
        public bool IsEnd;
    }

    private readonly Node _root = new();

    public void Insert(string word)
    {
        var cur = _root;
        foreach (char c in word)
        {
            if (!cur.Children.ContainsKey(c))
                cur.Children[c] = new Node();
            cur = cur.Children[c];
        }
        cur.IsEnd = true;
    }

    public bool Search(string word)
    {
        var node = Find(_root, word);
        return node is not null && node.IsEnd;
    }

    public bool StartsWith(string prefix) => Find(_root, prefix) is not null;

    public IEnumerable<string> GetWordsWithPrefix(string prefix)
    {
        var node = Find(_root, prefix);
        if (node is null) return [];
        var results = new List<string>();
        CollectWords(node, prefix, results);
        return results;
    }

    public void Delete(string word)
    {
        DeleteRec(_root, word, 0);
    }

    private static Node? Find(Node node, string s)
    {
        foreach (char c in s)
        {
            if (!node.Children.TryGetValue(c, out var next)) return null;
            node = next;
        }
        return node;
    }

    private static void CollectWords(Node node, string prefix, List<string> results)
    {
        if (node.IsEnd) results.Add(prefix);
        foreach (var (c, child) in node.Children)
            CollectWords(child, prefix + c, results);
    }

    private static bool DeleteRec(Node node, string word, int depth)
    {
        if (depth == word.Length) { node.IsEnd = false; return node.Children.Count == 0; }
        char c = word[depth];
        if (!node.Children.TryGetValue(c, out var child)) return false;
        if (DeleteRec(child, word, depth + 1))
            node.Children.Remove(c);
        return !node.IsEnd && node.Children.Count == 0;
    }
}

// ── Graph<T> ──────────────────────────────────────────────────────────────

public sealed class Graph<T> where T : notnull
{
    private readonly Dictionary<T, HashSet<T>> _adj = new();
    private readonly bool _directed;

    public Graph(bool directed = false) => _directed = directed;

    public void AddVertex(T v) { if (!_adj.ContainsKey(v)) _adj[v] = new HashSet<T>(); }

    public void AddEdge(T from, T to)
    {
        AddVertex(from); AddVertex(to);
        _adj[from].Add(to);
        if (!_directed) _adj[to].Add(from);
    }

    public IEnumerable<T> BFS(T start)
    {
        var visited = new HashSet<T>();
        var queue   = new Queue<T>();
        var result  = new List<T>();
        queue.Enqueue(start); visited.Add(start);
        while (queue.Count > 0)
        {
            var v = queue.Dequeue();
            result.Add(v);
            foreach (var nb in _adj.GetValueOrDefault(v, new()))
                if (visited.Add(nb)) queue.Enqueue(nb);
        }
        return result;
    }

    public IEnumerable<T> DFS(T start)
    {
        var visited = new HashSet<T>();
        var result  = new List<T>();
        DFSRec(start, visited, result);
        return result;
    }

    private void DFSRec(T v, HashSet<T> visited, List<T> result)
    {
        visited.Add(v); result.Add(v);
        foreach (var nb in _adj.GetValueOrDefault(v, new()))
            if (!visited.Contains(nb)) DFSRec(nb, visited, result);
    }

    public List<List<T>> ConnectedComponents()
    {
        var visited = new HashSet<T>();
        var result  = new List<List<T>>();
        foreach (var v in _adj.Keys)
        {
            if (visited.Contains(v)) continue;
            var comp = new List<T>();
            var q    = new Queue<T>([v]);
            visited.Add(v);
            while (q.Count > 0)
            {
                var cur = q.Dequeue(); comp.Add(cur);
                foreach (var nb in _adj.GetValueOrDefault(cur, new()))
                    if (visited.Add(nb)) q.Enqueue(nb);
            }
            result.Add(comp);
        }
        return result;
    }

    // Kahn's Algorithm (위상 정렬, DAG 전용)
    public List<T> TopologicalSort()
    {
        var inDeg  = _adj.Keys.ToDictionary(v => v, _ => 0);
        foreach (var (_, nbs) in _adj)
            foreach (var nb in nbs)
                inDeg[nb]++;

        var queue  = new Queue<T>(_adj.Keys.Where(v => inDeg[v] == 0));
        var result = new List<T>();
        while (queue.Count > 0)
        {
            var v = queue.Dequeue();
            result.Add(v);
            foreach (var nb in _adj.GetValueOrDefault(v, new()))
                if (--inDeg[nb] == 0) queue.Enqueue(nb);
        }
        return result;
    }
}

// ── WeightedGraph<T> (Dijkstra) ───────────────────────────────────────────

public sealed class WeightedGraph<T> where T : notnull
{
    private readonly Dictionary<T, List<(T To, double W)>> _adj = new();

    public void AddEdge(T from, T to, double weight)
    {
        if (!_adj.ContainsKey(from)) _adj[from] = new();
        if (!_adj.ContainsKey(to))   _adj[to]   = new();
        _adj[from].Add((to, weight));
        _adj[to].Add((from, weight));  // 무방향
    }

    public (double Distance, List<T> Path) Dijkstra(T start, T end)
    {
        var dist  = _adj.Keys.ToDictionary(v => v, _ => double.PositiveInfinity);
        var prev  = new Dictionary<T, T?>();
        var pq    = new PriorityQueue<T, double>();

        dist[start] = 0;
        pq.Enqueue(start, 0);

        while (pq.Count > 0)
        {
            var v = pq.Dequeue();
            if (v!.Equals(end)) break;

            foreach (var (nb, w) in _adj.GetValueOrDefault(v, new()))
            {
                double alt = dist[v] + w;
                if (alt < dist[nb])
                {
                    dist[nb]  = alt;
                    prev[nb]  = v;
                    pq.Enqueue(nb, alt);
                }
            }
        }

        // 경로 복원
        var path = new List<T>();
        for (T? cur = end; cur is not null; prev.TryGetValue(cur, out cur))
            path.Insert(0, cur);

        return (dist[end], path);
    }
}
