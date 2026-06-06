namespace Containers.Demos;

// ────────────────────────────────────────────────────────────────────────────
// 직접 구현하는 제네릭 컨테이너
//   ArrayStack<T>    — 동적 배열 기반 스택 (두 배 성장 전략)
//   CircularQueue<T> — 원형 버퍼 큐 (head/tail 포인터)
//   MinHeap<T>       — 이진 최소 힙 (우선순위 큐 내부 구조)
//   Deque<T>         — 양방향 큐 (앞/뒤 O(1) 삽입·삭제)
// ────────────────────────────────────────────────────────────────────────────
static class D2_CustomContainers
{
    public static void Run()
    {
        Print.Header("2. 직접 구현하는 제네릭 컨테이너");

        ShowArrayStack();
        ShowCircularQueue();
        ShowMinHeap();
        ShowDeque();
    }

    static void ShowArrayStack()
    {
        Print.Section("2-1. ArrayStack<T> — 동적 배열 스택 (두 배 성장)");

        var stack = new ArrayStack<int>();
        for (int i = 1; i <= 6; i++) stack.Push(i * 10);
        Print.Line($"Push(10~60) → Count={stack.Count}  Capacity={stack.Capacity}");
        Print.Line($"Peek: {stack.Peek()}");

        var popped = new List<int>();
        while (stack.Count > 0) popped.Add(stack.Pop());
        Print.Items(popped, "Pop 순서(LIFO): ");

        // TryPop 빈 스택
        Print.Line($"TryPop 빈 스택: {stack.TryPop(out _)}");

        // foreach
        foreach (int i in new[] { 5, 4, 3, 2, 1 }) stack.Push(i);
        Print.Items(stack, "foreach(top→bottom): ");
    }

    static void ShowCircularQueue()
    {
        Print.Section("2-2. CircularQueue<T> — 원형 버퍼 큐 (고정 용량)");

        var q = new CircularQueue<string>(capacity: 4);
        q.Enqueue("A"); q.Enqueue("B"); q.Enqueue("C");
        Print.Line($"Enqueue ABC → Count={q.Count}  IsFull={q.IsFull}");
        Print.Line($"Peek: {q.Peek()}");

        Print.Line($"Dequeue: {q.Dequeue()}  Count={q.Count}");
        q.Enqueue("D"); q.Enqueue("E");
        Print.Line($"Enqueue DE → IsFull={q.IsFull}");

        // 가득 찬 상태에서 Enqueue → 예외
        try { q.Enqueue("F"); }
        catch (InvalidOperationException e) { Print.Line($"Full 예외: {e.Message}"); }

        var items = new List<string>();
        while (q.Count > 0) items.Add(q.Dequeue());
        Print.Items(items, "FIFO 순서: ");
    }

    static void ShowMinHeap()
    {
        Print.Section("2-3. MinHeap<T> — 이진 최소 힙");

        var heap = new MinHeap<int>();
        int[] vals = [5, 3, 8, 1, 9, 2, 7, 4, 6];
        foreach (int v in vals) heap.Push(v);
        Print.Line($"Push {string.Join(",", vals)} → Count={heap.Count}  Peek={heap.Peek()}");

        // ExtractMin 순서 = 정렬 순서 (힙 정렬)
        var sorted = new List<int>();
        while (heap.Count > 0) sorted.Add(heap.Pop());
        Print.Items(sorted, "ExtractMin 순서: ");

        // IComparer 주입 — 최대 힙
        var maxHeap = new MinHeap<int>(Comparer<int>.Create((a, b) => b.CompareTo(a)));
        foreach (int v in vals) maxHeap.Push(v);
        var descSorted = new List<int>();
        while (maxHeap.Count > 0) descSorted.Add(maxHeap.Pop());
        Print.Items(descSorted, "MaxHeap(내림차순): ");

        // 커스텀 타입
        var taskHeap = new MinHeap<WorkItem>(Comparer<WorkItem>.Create((a, b) => a.Priority.CompareTo(b.Priority)));
        taskHeap.Push(new WorkItem(3, "보통"));
        taskHeap.Push(new WorkItem(1, "긴급"));
        taskHeap.Push(new WorkItem(2, "높음"));
        while (taskHeap.Count > 0)
        {
            var t = taskHeap.Pop();
            Console.Write($"      [{t.Priority}]{t.Name}  ");
        }
        Console.WriteLine();
    }

    static void ShowDeque()
    {
        Print.Section("2-4. Deque<T> — 양방향 큐 (앞/뒤 O(1))");

        var deque = new Deque<int>();
        deque.PushBack(3);
        deque.PushBack(4);
        deque.PushFront(2);
        deque.PushFront(1);
        deque.PushBack(5);

        Print.Line($"PushFront(1,2) + PushBack(3,4,5) → Count={deque.Count}");
        Print.Line($"PeekFront={deque.PeekFront()}  PeekBack={deque.PeekBack()}");

        var items = new List<int>();
        while (deque.Count > 0) items.Add(deque.PopFront());
        Print.Items(items, "PopFront(FIFO): ");

        // 슬라이딩 윈도우 최대값 예시
        int[] arr = [1, 3, -1, -3, 5, 3, 6, 7];
        int k = 3;
        Print.Items(SlidingWindowMax(arr, k), $"SlidingWindowMax(k={k}): ");
    }

    // 슬라이딩 윈도우 최대값 (단조 큐 Deque 활용)
    static IEnumerable<int> SlidingWindowMax(int[] arr, int k)
    {
        var dq = new Deque<int>(); // 인덱스 저장
        var result = new List<int>();
        for (int i = 0; i < arr.Length; i++)
        {
            while (dq.Count > 0 && dq.PeekBack() < i - k + 1) dq.PopBack();
            while (dq.Count > 0 && arr[dq.PeekBack()] < arr[i]) dq.PopBack();
            dq.PushBack(i);
            if (i >= k - 1) result.Add(arr[dq.PeekFront()]);
        }
        return result;
    }
}

// ── ArrayStack<T> ─────────────────────────────────────────────────────────

public sealed class ArrayStack<T> : IEnumerable<T>
{
    private T[] _items;
    private int _top;

    public ArrayStack(int initialCapacity = 4)
    {
        _items = new T[initialCapacity];
        _top   = 0;
    }

    public int Count    => _top;
    public int Capacity => _items.Length;
    public bool IsEmpty => _top == 0;

    public void Push(T item)
    {
        if (_top == _items.Length) Grow();
        _items[_top++] = item;
    }

    public T Pop()
    {
        if (IsEmpty) throw new InvalidOperationException("Stack이 비어 있습니다.");
        T item = _items[--_top];
        _items[_top] = default!;  // GC 루트 해제
        return item;
    }

    public bool TryPop(out T item)
    {
        if (IsEmpty) { item = default!; return false; }
        item = Pop();
        return true;
    }

    public T Peek() => IsEmpty
        ? throw new InvalidOperationException("Stack이 비어 있습니다.")
        : _items[_top - 1];

    public void Clear() { Array.Clear(_items, 0, _top); _top = 0; }

    private void Grow()
    {
        var newItems = new T[_items.Length * 2];
        Array.Copy(_items, newItems, _top);
        _items = newItems;
    }

    // top→bottom 순서로 열거
    public IEnumerator<T> GetEnumerator()
    {
        for (int i = _top - 1; i >= 0; i--) yield return _items[i];
    }
    System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() => GetEnumerator();
}

// ── CircularQueue<T> ─────────────────────────────────────────────────────

public sealed class CircularQueue<T>
{
    private readonly T[] _buf;
    private int _head, _tail, _count;

    public CircularQueue(int capacity)
    {
        _buf  = new T[capacity];
        _head = _tail = _count = 0;
    }

    public int  Count    => _count;
    public int  Capacity => _buf.Length;
    public bool IsEmpty  => _count == 0;
    public bool IsFull   => _count == _buf.Length;

    public void Enqueue(T item)
    {
        if (IsFull) throw new InvalidOperationException("큐가 가득 찼습니다.");
        _buf[_tail] = item;
        _tail = (_tail + 1) % _buf.Length;
        _count++;
    }

    public T Dequeue()
    {
        if (IsEmpty) throw new InvalidOperationException("큐가 비어 있습니다.");
        T item = _buf[_head];
        _buf[_head] = default!;
        _head = (_head + 1) % _buf.Length;
        _count--;
        return item;
    }

    public bool TryDequeue(out T item)
    {
        if (IsEmpty) { item = default!; return false; }
        item = Dequeue();
        return true;
    }

    public T Peek() => IsEmpty
        ? throw new InvalidOperationException("큐가 비어 있습니다.")
        : _buf[_head];
}

// ── MinHeap<T> ───────────────────────────────────────────────────────────

public sealed class MinHeap<T>
{
    private readonly List<T> _heap;
    private readonly IComparer<T> _cmp;

    public MinHeap(IComparer<T>? comparer = null)
    {
        _heap = new List<T>();
        _cmp  = comparer ?? Comparer<T>.Default;
    }

    public int Count => _heap.Count;
    public T Peek()  => _heap.Count == 0
        ? throw new InvalidOperationException("Heap이 비어 있습니다.")
        : _heap[0];

    public void Push(T item)
    {
        _heap.Add(item);
        BubbleUp(_heap.Count - 1);
    }

    public T Pop()
    {
        if (_heap.Count == 0) throw new InvalidOperationException("Heap이 비어 있습니다.");
        T min = _heap[0];
        int last = _heap.Count - 1;
        _heap[0] = _heap[last];
        _heap.RemoveAt(last);
        if (_heap.Count > 0) SiftDown(0);
        return min;
    }

    private void BubbleUp(int i)
    {
        while (i > 0)
        {
            int parent = (i - 1) / 2;
            if (_cmp.Compare(_heap[i], _heap[parent]) >= 0) break;
            (_heap[i], _heap[parent]) = (_heap[parent], _heap[i]);
            i = parent;
        }
    }

    private void SiftDown(int i)
    {
        int n = _heap.Count;
        while (true)
        {
            int smallest = i, l = 2 * i + 1, r = 2 * i + 2;
            if (l < n && _cmp.Compare(_heap[l], _heap[smallest]) < 0) smallest = l;
            if (r < n && _cmp.Compare(_heap[r], _heap[smallest]) < 0) smallest = r;
            if (smallest == i) break;
            (_heap[i], _heap[smallest]) = (_heap[smallest], _heap[i]);
            i = smallest;
        }
    }
}

// ── Deque<T> (Doubly-ended Queue) ────────────────────────────────────────

public sealed class Deque<T>
{
    private T[] _buf;
    private int _front, _back, _count;

    public Deque(int capacity = 8)
    {
        _buf   = new T[capacity];
        _front = capacity / 2;
        _back  = capacity / 2;
    }

    public int  Count   => _count;
    public bool IsEmpty => _count == 0;

    public void PushFront(T item)
    {
        if (_front == 0) Grow();
        _buf[--_front] = item;
        _count++;
    }

    public void PushBack(T item)
    {
        if (_back == _buf.Length) Grow();
        _buf[_back++] = item;
        _count++;
    }

    public T PopFront()
    {
        if (IsEmpty) throw new InvalidOperationException();
        T item = _buf[_front];
        _buf[_front++] = default!;
        _count--;
        return item;
    }

    public T PopBack()
    {
        if (IsEmpty) throw new InvalidOperationException();
        T item = _buf[--_back];
        _buf[_back] = default!;
        _count--;
        return item;
    }

    public T PeekFront() => _buf[_front];
    public T PeekBack()  => _buf[_back - 1];

    private void Grow()
    {
        var newBuf = new T[_buf.Length * 2];
        int newFront = _buf.Length / 2;
        Array.Copy(_buf, _front, newBuf, newFront, _count);
        _front = newFront;
        _back  = newFront + _count;
        _buf   = newBuf;
    }
}

// ── 보조 타입 ─────────────────────────────────────────────────────────────

public record WorkItem(int Priority, string Name);
