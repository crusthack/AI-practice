namespace TypeSystem.Demos;

static class D3_Records
{
    public static void Run()
    {
        Print.Header("3. Record 타입 — 값 동등성 + 불변성");

        // ── 3-1. Positional record vs class
        Print.Section("3-1. record class — 값 동등성 (class와 비교)");
        {
            // class: 참조 동등성
            var pc1 = new PersonClass("Alice", 30);
            var pc2 = new PersonClass("Alice", 30);
            Console.WriteLine($"    class:  pc1==pc2 → {pc1 == pc2}  (같은 내용, 다른 주소 → false)");

            // record: 값 동등성 (모든 프로퍼티를 비교)
            var pr1 = new PersonRecord("Alice", 30);
            var pr2 = new PersonRecord("Alice", 30);
            Console.WriteLine($"    record: pr1==pr2 → {pr1 == pr2}  (내용 비교 → true)");
            Console.WriteLine($"    ReferenceEquals: {ReferenceEquals(pr1, pr2)}  (다른 인스턴스)");
        }

        // ── 3-2. with 표현식 + 구조 분해
        Print.Section("3-2. with 표현식 & Deconstruct");
        {
            var p1 = new PersonRecord("Alice", 30);
            var p2 = p1 with { Age = 31 };   // 새 인스턴스, Age만 변경
            var p3 = p1 with { Name = "Bob", Age = 25 };

            Console.WriteLine($"    원본: {p1}");
            Console.WriteLine($"    나이+1: {p2}");
            Console.WriteLine($"    새 사람: {p3}");

            // Deconstruct 자동 생성
            var (name, age) = p1;
            Console.WriteLine($"    분해: name={name}, age={age}");
        }

        // ── 3-3. Static factory + 유효성 검사 패턴
        Print.Section("3-3. Static factory — 생성 시점 유효성 검사");
        {
            var ok = Product.Create("P001", "무선 마우스", 29_900m);
            Console.WriteLine($"    {ok.DisplayName}  {ok.Price:C}");
            Console.WriteLine($"    10% 할인: {ok.WithDiscount(0.10).Price:C}");

            // 잘못된 입력 → ArgumentException
            try { _ = Product.Create("", "bad", -1m); }
            catch (ArgumentException ex)
            {
                Console.WriteLine($"    빈 Id → {ex.GetType().Name}: {ex.ParamName}");
            }
            try { _ = Product.Create("P2", "item", -100m); }
            catch (ArgumentOutOfRangeException ex)
            {
                Console.WriteLine($"    음수 가격 → {ex.GetType().Name}: {ex.ParamName}");
            }
        }

        // ── 3-4. Non-positional record + required init
        Print.Section("3-4. Non-positional record + required init");
        {
            var addr = new Address { Street = "세종대로 1", City = "서울" };
            Console.WriteLine($"    {addr.Full}");

            // with로 City만 변경
            var addr2 = addr with { City = "부산" };
            Console.WriteLine($"    with 부산: {addr2.Full}");

            // 동등성
            var addr3 = new Address { Street = "세종대로 1", City = "서울" };
            Console.WriteLine($"    addr == addr3: {addr == addr3}");
        }

        // ── 3-5. Record 상속
        Print.Section("3-5. Record 상속 — 타입 기반 동등성");
        {
            Animal animal = new Animal("Buddy", "Canis lupus familiaris");
            Dog    dog    = new Dog("Buddy", "리트리버");

            Console.WriteLine($"    animal: {animal}");
            Console.WriteLine($"    dog:    {dog}");
            Console.WriteLine($"    짖기:   {dog.Bark()}");

            // 타입이 다르면 Equals = false (EqualityContract 비교)
            Console.WriteLine($"    animal == dog: {animal == dog}  (타입 달라 false)");

            // is 패턴 매칭
            if (animal is Dog d)
                Console.WriteLine($"    Dog로 캐스트 성공: {d.Bark()}");
            else
                Console.WriteLine($"    animal은 Dog 아님 (Animal 인스턴스)");
        }

        // ── 3-6. sealed record + sealed Point2D
        Print.Section("3-6. sealed record — 상속 차단 + 값 동등성");
        {
            var p1 = new Point2D(0, 0);
            var p2 = new Point2D(3, 4);
            Console.WriteLine($"    Origin={Point2D.Origin}");
            Console.WriteLine($"    p2={p2}  거리={p2.Distance:F2}");
            Console.WriteLine($"    p1→p2 거리: {p1.DistanceTo(p2):F2}");
            Console.WriteLine($"    Translate(1,1): {p2.Translate(1, 1)}");
        }

        // ── 3-7. record struct vs record class — 메모리 배치
        Print.Section("3-7. record struct (값) vs record class (참조)");
        {
            // record struct → 스택 할당, 복사 의미론
            var v1 = new Velocity(3.0, 4.0);
            var v2 = v1;          // 값 복사
            v2 = v2.Scale(2.0);
            Console.WriteLine($"    v1={v1}  속도={v1.Speed:F2}");
            Console.WriteLine($"    v2(2배)={v2}  (v1 변경 없음)");

            // readonly record struct
            Console.WriteLine($"    Red={RGB.Red.Hex}  White={RGB.White.Hex}");
            Console.WriteLine($"    Red.Blend(White)={RGB.Red.Blend(RGB.White).Hex}");
            Console.WriteLine($"    Red.Invert()={RGB.Red.Invert().Hex}");

            // record class → 힙 할당, 참조 의미론
            var pr1 = new PersonRecord("Alice", 30);
            var pr2 = pr1;    // 참조 복사 (같은 객체)
            Console.WriteLine($"    record class: ReferenceEquals={ReferenceEquals(pr1, pr2)}  " +
                              $"Equals={pr1 == pr2}");
        }
    }
}
