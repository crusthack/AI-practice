using Consumer;

// AutoToStringGenerator가 생성한 ToString() 메서드 사용
var product = new Product { Name = "Laptop", Price = 999.99m, Stock = 5 };
Console.WriteLine(product);  // Product(Name=Laptop, Price=999.99, Stock=5)

var order = new Order { Id = 42, Customer = "홍길동", Status = "Shipped" };
Console.WriteLine(order);    // Order(Id=42, Customer=홍길동, Status=Shipped)

// 빌드 시 EMPTYCAT001 경고가 ServiceWithIssues.ProcessOrder에서 발생합니다.
// dotnet build 출력에서 확인 가능.
