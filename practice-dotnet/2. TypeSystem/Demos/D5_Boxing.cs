namespace TypeSystem.Demos;

static class D5_Boxing
{
    public static void Run()
    {
        Print.Header("5. 박싱/언박싱 & 스택 vs 힙 메모리");

        // ── 5-1. 박싱/언박싱 기초
        Print.Section("5-1. 박싱(Boxing) / 언박싱(Unboxing)");
        {
            int i = 42;

            // 박싱: 값 타입 → object (힙에 새 객체 생성 + 복사)
            object boxed = i;

            // 언박싱: object → 값 타입 (힙에서 꺼내 스택으로 복사)
            int unboxed = (int)boxed;

            Console.WriteLine($"    원본={i}  박싱={boxed}  언박싱={unboxed}");
            Console.WriteLine($"    ReferenceEquals(i, boxed): {ReferenceEquals(i, boxed)}  (별개 인스턴스)");

            // 박싱 후 원본 변경해도 박싱된 값에 영향 없음
            i = 999;
            Console.WriteLine($"    i=999 변경 후 boxed={boxed}  (독립 복사본)");

            // 잘못된 타입으로 언박싱 → InvalidCastException
            try { long wrong = (long)boxed; }
            catch (InvalidCastException e) { Console.WriteLine($"    잘못된 언박싱: {e.GetType().Name}"); }
        }

        // ── 5-2. 박싱이 발생하는 패턴들
        Print.Section("5-2. 박싱 발생 패턴");
        {
            // 패턴 1: 비제네릭 컬렉션 (ArrayList)
            var list = new System.Collections.ArrayList();
            list.Add(1);   // int → object 박싱
            list.Add(2);
            int sum1 = (int)list[0]! + (int)list[1]!;  // 언박싱
            Console.WriteLine($"    ArrayList 합: {sum1}  (모두 박싱/언박싱)");

            // 패턴 2: string.Format — 값 타입 인수 박싱
            int x = 42;
            string s1 = string.Format("값={0}", x);     // x 박싱 발생
            string s2 = $"값={x}";                      // 박싱 없음 (컴파일러 최적화)
            Console.WriteLine($"    Format: {s1}  Interpolation: {s2}");

            // 패턴 3: 인터페이스 변수에 struct 대입
            IComparable cmp = 42;          // int → IComparable (박싱)
            Console.WriteLine($"    IComparable: {cmp.CompareTo(41) > 0}  (박싱됨)");

            // 패턴 4: Delegate / Event에 값 타입 클로저 캡처
            int captured = 10;
            Action act = () => Console.WriteLine($"    캡처된 값: {captured}");
            act();  // captured는 힙의 클로저 객체로 승격됨
        }

        // ── 5-3. 박싱 회피 패턴
        Print.Section("5-3. 박싱 회피 — Generic / IEquatable<T> / Span<T>");
        {
            // 회피 1: 제네릭 컬렉션 → 박싱 없음
            var generic = new List<int> { 1, 2, 3, 4, 5 };
            Console.WriteLine($"    List<int> 합: {generic.Sum()}  (박싱 없음)");

            // 회피 2: IEquatable<T> 직접 호출 → 박싱 없이 값 비교
            var p1 = new ImmutablePoint(1, 2);
            var p2 = new ImmutablePoint(1, 2);
            bool eq = p1.Equals(p2);   // IEquatable<ImmutablePoint> — 박싱 없음
            Console.WriteLine($"    IEquatable.Equals: {eq}  (박싱 없음)");

            // 회피 3: Generic 메서드 — 컴파일 타임에 T 특수화
            Console.WriteLine($"    Generic Max: {GenericMax(3, 7)}  (박싱 없음)");

            // 회피 4: Span<T> — 배열을 박싱 없이 처리
            int[] arr = [10, 20, 30, 40, 50];
            Console.WriteLine($"    Span<int> 합: {SumSpan(arr)}  (박싱 없음)");
        }

        // ── 5-4. struct vs class — 메모리 위치
        Print.Section("5-4. 스택 vs 힙 — 메모리 배치");
        {
            // struct: 로컬 변수 → 스택 (크기 고정, GC 없음)
            MutablePoint stackPt = new(1, 2);   // 스택

            // class: 항상 힙 (GC 추적)
            PersonClass heapObj = new("Alice", 30);  // 힙

            Console.WriteLine($"    struct (스택): {stackPt}");
            Console.WriteLine($"    class  (힙):   {heapObj}");

            // struct 배열: 배열 헤더는 힙, 요소는 연속 인라인 저장 (캐시 친화적)
            MutablePoint[] pts = [new(1,2), new(3,4), new(5,6)];
            double distSum = 0;
            foreach (ref var pt in pts.AsSpan())  // ref foreach → 복사 없음
                distSum += pt.Distance;
            Console.WriteLine($"    struct 배열 거리 합: {distSum:F2}  (연속 메모리, 캐시 효율적)");

            // class 배열: 요소는 참조(포인터) — 각 객체는 힙 어딘가에 흩어짐
            PersonClass[] people = [new("A", 1), new("B", 2), new("C", 3)];
            Console.WriteLine($"    class 배열: {people.Length}개 참조 (각 객체는 힙에 개별 할당)");
        }

        // ── 5-5. Nullable 값 타입
        Print.Section("5-5. Nullable<T> — 값 타입의 null 표현");
        {
            int? a = null;
            int? b = 42;

            Console.WriteLine($"    a={a?.ToString() ?? "null"}  b={b}");
            Console.WriteLine($"    a.HasValue={a.HasValue}  b.HasValue={b.HasValue}");
            Console.WriteLine($"    null 합병: a ?? -1 = {a ?? -1}");
            Console.WriteLine($"    null 조건: b?.ToString() = {b?.ToString()}");
            Console.WriteLine($"    GetValueOrDefault: a={a.GetValueOrDefault()}  b={b.GetValueOrDefault()}");

            // Nullable<T>는 실제로는 두 필드로 구성된 struct
            // struct { T _value; bool _hasValue; }
            // null인 Nullable에 GetType() 호출 시 박싱 → null 참조 → NullReferenceException
            // typeof()는 런타임 인스턴스가 필요없으므로 안전
            Console.WriteLine($"    sizeof(int)={System.Runtime.InteropServices.Marshal.SizeOf<int>()}  " +
                              $"Nullable<int> 타입: {typeof(int?).Name}  (내부: struct{{int,bool}})");
        }

        // ── 5-6. Nullable 참조 타입 (C# 8+)
        Print.Section("5-6. Nullable 참조 타입 — 컴파일 타임 null 분석");
        {
            // #nullable enable 환경에서
            string  nonNull  = "hello";    // null 불가
            string? nullable = null;       // null 허용

            Console.WriteLine($"    nonNull: {nonNull}");
            Console.WriteLine($"    nullable: {nullable ?? "(null)"}");

            // null 조건 접근
            // null-conditional: nullable?.Length는 int? 를 반환
            // int (비-nullable) 에는 ?. 적용 불가 → 괄호로 묶어 int? 로 만들어야 함
            Console.WriteLine($"    nullable?.Length = {(nullable?.Length)?.ToString() ?? "null"}");

            // ! 연산자: null이 아님을 단언 (위험 — 확신할 때만)
            nullable = "world";
            Console.WriteLine($"    !연산자: {nullable!.ToUpper()}");
        }
    }

    static T GenericMax<T>(T a, T b) where T : IComparable<T> =>
        a.CompareTo(b) >= 0 ? a : b;

    static int SumSpan(ReadOnlySpan<int> span)
    {
        int s = 0;
        foreach (var v in span) s += v;
        return s;
    }
}
