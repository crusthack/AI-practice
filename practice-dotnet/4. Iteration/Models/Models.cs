namespace Iteration;

public record Student(string Name, int Grade, double Gpa, string Department)
    : IComparable<Student>
{
    public int CompareTo(Student? other) =>
        other is null ? 1 : Gpa.CompareTo(other.Gpa);

    public override string ToString() => $"{Name}({Department}, {Grade}학년, GPA={Gpa:F1})";
}

public record Product(string Name, string Category, decimal Price, int Stock)
{
    public bool IsAvailable => Stock > 0;
    public override string ToString() => $"{Name}[{Category}] {Price:C} ({Stock}개)";
}

public record OrderItem(string ProductName, decimal Price, int Quantity)
{
    public decimal Subtotal => Price * Quantity;
}
