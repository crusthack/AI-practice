using Containers.Demos;
using System.Collections.Frozen;

// ── ArrayStack ─────────────────────────────────────────────────────────────
public class ArrayStackTests
{
    [Fact] public void Push_Pop_LIFO()
    {
        var s = new ArrayStack<int>();
        s.Push(1); s.Push(2); s.Push(3);
        Assert.Equal(3, s.Pop()); Assert.Equal(2, s.Pop()); Assert.Equal(1, s.Pop());
    }
    [Fact] public void Grows_Automatically()
    {
        var s = new ArrayStack<int>(2);
        for (int i = 0; i < 10; i++) s.Push(i);
        Assert.Equal(10, s.Count);
        Assert.True(s.Capacity >= 10);
    }
    [Fact] public void Pop_Empty_Throws() => Assert.Throws<InvalidOperationException>(() => new ArrayStack<int>().Pop());
    [Fact] public void TryPop_Empty_ReturnsFalse() => Assert.False(new ArrayStack<int>().TryPop(out _));
    [Fact] public void Foreach_TopToBottom()
    {
        var s = new ArrayStack<int>(); s.Push(1); s.Push(2); s.Push(3);
        Assert.Equal(new[] { 3, 2, 1 }, s.ToArray());
    }
}

// ── CircularQueue ──────────────────────────────────────────────────────────
public class CircularQueueTests
{
    [Fact] public void Enqueue_Dequeue_FIFO()
    {
        var q = new CircularQueue<int>(4);
        q.Enqueue(1); q.Enqueue(2); q.Enqueue(3);
        Assert.Equal(1, q.Dequeue()); Assert.Equal(2, q.Dequeue());
    }
    [Fact] public void Full_Queue_Throws()
    {
        var q = new CircularQueue<int>(2);
        q.Enqueue(1); q.Enqueue(2);
        Assert.True(q.IsFull);
        Assert.Throws<InvalidOperationException>(() => q.Enqueue(3));
    }
    [Fact] public void Wrap_Around_Works()
    {
        var q = new CircularQueue<int>(3);
        q.Enqueue(1); q.Enqueue(2); q.Enqueue(3);
        q.Dequeue(); q.Dequeue();
        q.Enqueue(4); q.Enqueue(5);  // 랩어라운드
        Assert.Equal(3, q.Dequeue()); Assert.Equal(4, q.Dequeue()); Assert.Equal(5, q.Dequeue());
    }
}

// ── MinHeap ────────────────────────────────────────────────────────────────
public class MinHeapTests
{
    [Fact] public void Extracts_In_Sorted_Order()
    {
        var h = new MinHeap<int>();
        foreach (int v in new[] { 5, 3, 8, 1, 9, 2 }) h.Push(v);
        var result = new List<int>();
        while (h.Count > 0) result.Add(h.Pop());
        Assert.Equal(new[] { 1, 2, 3, 5, 8, 9 }, result);
    }
    [Fact] public void MaxHeap_Via_Comparer()
    {
        var h = new MinHeap<int>(Comparer<int>.Create((a, b) => b.CompareTo(a)));
        foreach (int v in new[] { 3, 1, 2 }) h.Push(v);
        Assert.Equal(3, h.Pop());
    }
    [Fact] public void Peek_Does_Not_Remove()
    {
        var h = new MinHeap<int>(); h.Push(5); h.Push(1);
        Assert.Equal(1, h.Peek()); Assert.Equal(2, h.Count);
    }
}

// ── Deque ──────────────────────────────────────────────────────────────────
public class DequeTests
{
    [Fact] public void PushFront_Back_And_Pop()
    {
        var d = new Deque<int>();
        d.PushBack(2); d.PushFront(1); d.PushBack(3);
        Assert.Equal(1, d.PopFront()); Assert.Equal(3, d.PopBack()); Assert.Equal(2, d.PopFront());
    }
    [Fact] public void Grows_When_Full()
    {
        var d = new Deque<int>(4);
        for (int i = 0; i < 20; i++) d.PushBack(i);
        Assert.Equal(20, d.Count);
    }
}

// ── BST ────────────────────────────────────────────────────────────────────
public class BSTTests
{
    BinarySearchTree<int> Build(params int[] vals)
    {
        var t = new BinarySearchTree<int>();
        foreach (var v in vals) t.Insert(v);
        return t;
    }

    [Fact] public void InOrder_Is_Sorted()
        => Assert.Equal(new[] { 1, 2, 3, 5, 7, 8, 9 }, Build(5, 3, 8, 1, 9, 2, 7).InOrder());

    [Fact] public void Contains() { var t = Build(5, 3, 7); Assert.True(t.Contains(3)); Assert.False(t.Contains(99)); }
    [Fact] public void MinMax()   { var t = Build(5, 3, 7, 1, 9); Assert.Equal(1, t.Min); Assert.Equal(9, t.Max); }
    [Fact] public void Remove_Leaf()
    {
        var t = Build(5, 3, 7); t.Remove(7);
        Assert.False(t.Contains(7)); Assert.Equal(2, t.Count);
    }
    [Fact] public void Remove_TwoChildren()
    {
        var t = Build(5, 3, 8, 1, 4, 7, 9); t.Remove(3);
        Assert.False(t.Contains(3)); Assert.Contains(4, t.InOrder());
    }
}

// ── Trie ───────────────────────────────────────────────────────────────────
public class TrieTests
{
    [Fact] public void Search_And_StartsWith()
    {
        var t = new Trie(); t.Insert("apple"); t.Insert("app");
        Assert.True(t.Search("apple")); Assert.True(t.Search("app"));
        Assert.False(t.Search("ap")); Assert.True(t.StartsWith("ap"));
    }
    [Fact] public void Autocomplete()
    {
        var t = new Trie();
        foreach (var w in new[] { "apple", "app", "apply", "apt", "banana" }) t.Insert(w);
        var result = t.GetWordsWithPrefix("app").OrderBy(x => x).ToList();
        Assert.Equal(new[] { "app", "apple", "apply" }, result);
    }
    [Fact] public void Delete_Word()
    {
        var t = new Trie(); t.Insert("apple"); t.Insert("app");
        t.Delete("apple");
        Assert.False(t.Search("apple")); Assert.True(t.Search("app")); Assert.True(t.StartsWith("app"));
    }
}

// ── FrozenDictionary / MultiMap ────────────────────────────────────────────
public class AdvancedCollectionTests
{
    [Fact] public void FrozenDictionary_Lookup()
    {
        var fd = new Dictionary<string, int> { ["a"] = 1, ["b"] = 2 }.ToFrozenDictionary();
        Assert.Equal(1, fd["a"]); Assert.True(fd.ContainsKey("b")); Assert.False(fd.ContainsKey("c"));
    }
    [Fact] public void MultiMap_AddAndQuery()
    {
        var m = new MultiMap<string, int>();
        m.Add("x", 1); m.Add("x", 2); m.Add("y", 3);
        Assert.Equal(2, m["x"].Count); Assert.True(m.ContainsValue("x", 2));
    }
    [Fact] public void MultiMap_Remove()
    {
        var m = new MultiMap<string, int>(); m.Add("k", 1); m.Add("k", 2);
        m.Remove("k", 1);
        Assert.Single(m["k"]); Assert.False(m.ContainsValue("k", 1));
    }
}

// ── Patterns ────────────────────────────────────────────────────────────────
public class PatternTests
{
    [Fact] public void ObjectPool_ReusesObjects()
    {
        var pool = new SimpleObjectPool<List<int>>(() => new List<int>(), r => r.Clear());
        var obj = pool.Rent(); obj.Add(42); pool.Return(obj);
        var reused = pool.Rent();
        Assert.Empty(reused);  // reset이 호출되어 비어있어야 함
    }
    [Fact] public void LazyCollection_InitOnceLazily()
    {
        int calls = 0;
        var lc = new LazyCollection<int>(() => { calls++; return new[] { 1, 2, 3 }; });
        Assert.False(lc.IsValueCreated); Assert.Equal(0, calls);
        _ = lc.Count; _ = lc.Count;
        Assert.Equal(1, calls);
    }
    [Fact] public void Point3DEqualityComparer_Epsilon()
    {
        var cmp = new Point3DEqualityComparer(0.01);
        Assert.True(cmp.Equals(new Point3D(1.0, 2.0, 3.0), new Point3D(1.005, 2.0, 3.0)));
        Assert.False(cmp.Equals(new Point3D(1.0, 2.0, 3.0), new Point3D(1.02,  2.0, 3.0)));
    }
}
