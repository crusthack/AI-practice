using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace Roslyn.Demos;

// SyntaxFactory로 C# AST를 프로그래매틱하게 구성
static class D4_SyntaxFactory
{
    public static void Run()
    {
        Print.Header("4. SyntaxFactory — 프로그래매틱 AST 구성 & 코드 생성");
        BuildSimpleClass();
        BuildRecord();
        BuildExtensionMethod();
        BuildWithDocComments();
        BuildFullSource();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void BuildSimpleClass()
    {
        Print.Section("4-1. 클래스 + 프로퍼티 + 생성자 + 메서드 빌드");

        // 프로퍼티: public string Name { get; set; }
        var nameProp = SyntaxFactory.PropertyDeclaration(
                SyntaxFactory.ParseTypeName("string"), "Name")
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword))
            .AddAccessorListAccessors(
                Accessor(SyntaxKind.GetAccessorDeclaration),
                Accessor(SyntaxKind.SetAccessorDeclaration));

        var ageProp = SyntaxFactory.PropertyDeclaration(
                SyntaxFactory.ParseTypeName("int"), "Age")
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword))
            .AddAccessorListAccessors(
                Accessor(SyntaxKind.GetAccessorDeclaration),
                Accessor(SyntaxKind.SetAccessorDeclaration));

        // 생성자: public Person(string name, int age) { Name = name; Age = age; }
        var ctor = SyntaxFactory.ConstructorDeclaration("Person")
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword))
            .AddParameterListParameters(
                Param("string", "name"),
                Param("int",    "age"))
            .WithBody(SyntaxFactory.Block(
                AssignStmt("Name", "name"),
                AssignStmt("Age",  "age")));

        // 메서드: public override string ToString() => $"Person(Name={Name}, Age={Age})";
        var toStrMethod = SyntaxFactory.MethodDeclaration(
                SyntaxFactory.ParseTypeName("string"), "ToString")
            .AddModifiers(
                SyntaxFactory.Token(SyntaxKind.PublicKeyword),
                SyntaxFactory.Token(SyntaxKind.OverrideKeyword))
            .WithExpressionBody(SyntaxFactory.ArrowExpressionClause(
                SyntaxFactory.ParseExpression(@"$""Person(Name={Name}, Age={Age})""")))
            .WithSemicolonToken(SyntaxFactory.Token(SyntaxKind.SemicolonToken));

        // 클래스 조립
        var classDecl = SyntaxFactory.ClassDeclaration("Person")
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword))
            .AddMembers(nameProp, ageProp, ctor, toStrMethod);

        var cu = SyntaxFactory.CompilationUnit()
            .AddMembers(classDecl)
            .NormalizeWhitespace();

        PrintSource(cu.ToFullString(), maxLines: 20);
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void BuildRecord()
    {
        Print.Section("4-2. Record 선언 빌드 (C# 9+)");

        // public record Point(double X, double Y);
        var pointRecord = SyntaxFactory.RecordDeclaration(
                SyntaxFactory.Token(SyntaxKind.RecordKeyword), "Point")
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword))
            .WithParameterList(SyntaxFactory.ParameterList(
                SyntaxFactory.SeparatedList(new[]
                {
                    SyntaxFactory.Parameter(SyntaxFactory.Identifier("X"))
                        .WithType(SyntaxFactory.ParseTypeName("double")),
                    SyntaxFactory.Parameter(SyntaxFactory.Identifier("Y"))
                        .WithType(SyntaxFactory.ParseTypeName("double")),
                })))
            .WithSemicolonToken(SyntaxFactory.Token(SyntaxKind.SemicolonToken));

        // public record struct ColoredPoint(double X, double Y, string Color) : Point(X, Y);
        var coloredRecord = SyntaxFactory.RecordDeclaration(
                SyntaxFactory.Token(SyntaxKind.RecordKeyword), "ColoredPoint")
            .AddModifiers(
                SyntaxFactory.Token(SyntaxKind.PublicKeyword),
                SyntaxFactory.Token(SyntaxKind.SealedKeyword))
            .WithParameterList(SyntaxFactory.ParameterList(
                SyntaxFactory.SeparatedList(new[]
                {
                    SyntaxFactory.Parameter(SyntaxFactory.Identifier("X"))
                        .WithType(SyntaxFactory.ParseTypeName("double")),
                    SyntaxFactory.Parameter(SyntaxFactory.Identifier("Y"))
                        .WithType(SyntaxFactory.ParseTypeName("double")),
                    SyntaxFactory.Parameter(SyntaxFactory.Identifier("Color"))
                        .WithType(SyntaxFactory.ParseTypeName("string")),
                })))
            .WithSemicolonToken(SyntaxFactory.Token(SyntaxKind.SemicolonToken));

        var cu = SyntaxFactory.CompilationUnit()
            .AddMembers(pointRecord, coloredRecord)
            .NormalizeWhitespace();

        PrintSource(cu.ToFullString(), maxLines: 15);
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void BuildExtensionMethod()
    {
        Print.Section("4-3. 인터페이스 + 확장 메서드 빌드");

        // interface ILogger { void Log(string message); }
        var ifaceMethod = SyntaxFactory.MethodDeclaration(
                SyntaxFactory.ParseTypeName("void"), "Log")
            .AddParameterListParameters(Param("string", "message"))
            .WithSemicolonToken(SyntaxFactory.Token(SyntaxKind.SemicolonToken));

        var iface = SyntaxFactory.InterfaceDeclaration("ILogger")
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword))
            .AddMembers(ifaceMethod);

        // static class LoggerExtensions
        // { static void LogInfo(this ILogger logger, string msg) => logger.Log($"[INFO] {msg}"); }
        var extMethod = SyntaxFactory.MethodDeclaration(
                SyntaxFactory.ParseTypeName("void"), "LogInfo")
            .AddModifiers(
                SyntaxFactory.Token(SyntaxKind.PublicKeyword),
                SyntaxFactory.Token(SyntaxKind.StaticKeyword))
            .AddParameterListParameters(
                SyntaxFactory.Parameter(SyntaxFactory.Identifier("logger"))
                    .WithType(SyntaxFactory.ParseTypeName("ILogger"))
                    .AddModifiers(SyntaxFactory.Token(SyntaxKind.ThisKeyword)),
                Param("string", "msg"))
            .WithExpressionBody(SyntaxFactory.ArrowExpressionClause(
                SyntaxFactory.ParseExpression(@"logger.Log($""[INFO] {msg}"")")))
            .WithSemicolonToken(SyntaxFactory.Token(SyntaxKind.SemicolonToken));

        var extClass = SyntaxFactory.ClassDeclaration("LoggerExtensions")
            .AddModifiers(
                SyntaxFactory.Token(SyntaxKind.PublicKeyword),
                SyntaxFactory.Token(SyntaxKind.StaticKeyword))
            .AddMembers(extMethod);

        var cu = SyntaxFactory.CompilationUnit()
            .AddMembers(iface, extClass)
            .NormalizeWhitespace();

        PrintSource(cu.ToFullString(), maxLines: 20);
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void BuildWithDocComments()
    {
        Print.Section("4-4. XML 문서 주석 Trivia 부착");

        // /// <summary>두 수를 더합니다.</summary>
        // /// <param name="a">첫 번째 수</param>
        // /// <param name="b">두 번째 수</param>
        // /// <returns>합</returns>
        // public static int Add(int a, int b) => a + b;

        var method = SyntaxFactory.MethodDeclaration(
                SyntaxFactory.ParseTypeName("int"), "Add")
            .AddModifiers(
                SyntaxFactory.Token(SyntaxKind.PublicKeyword),
                SyntaxFactory.Token(SyntaxKind.StaticKeyword))
            .AddParameterListParameters(Param("int", "a"), Param("int", "b"))
            .WithExpressionBody(SyntaxFactory.ArrowExpressionClause(
                SyntaxFactory.ParseExpression("a + b")))
            .WithSemicolonToken(SyntaxFactory.Token(SyntaxKind.SemicolonToken))
            .WithLeadingTrivia(
                SyntaxFactory.ParseLeadingTrivia(
                    "/// <summary>두 수를 더합니다.</summary>\n" +
                    "/// <param name=\"a\">첫 번째 수</param>\n" +
                    "/// <param name=\"b\">두 번째 수</param>\n" +
                    "/// <returns>합계</returns>\n"));

        var cls = SyntaxFactory.ClassDeclaration("MathUtils")
            .AddModifiers(
                SyntaxFactory.Token(SyntaxKind.PublicKeyword),
                SyntaxFactory.Token(SyntaxKind.StaticKeyword))
            .AddMembers(method);

        var cu = SyntaxFactory.CompilationUnit().AddMembers(cls).NormalizeWhitespace();
        PrintSource(cu.ToFullString(), maxLines: 15);

        // 기존 트리에서 XML trivia 파싱
        var existing = CSharpSyntaxTree.ParseText(cu.ToFullString());
        var xmlTrivia = existing.GetRoot().DescendantTrivia()
            .Where(t => t.IsKind(SyntaxKind.SingleLineDocumentationCommentTrivia))
            .ToList();
        Print.Line($"생성된 XML trivia: {xmlTrivia.Count}개");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void BuildFullSource()
    {
        Print.Section("4-5. 완전한 소스 파일 구성 (using + namespace + class)");

        // using 지시문
        var usings = new[]
        {
            SyntaxFactory.UsingDirective(SyntaxFactory.ParseName("System")),
            SyntaxFactory.UsingDirective(SyntaxFactory.ParseName("System.Collections.Generic")),
            SyntaxFactory.UsingDirective(SyntaxFactory.ParseName("System.Linq")),
        };

        // 제네릭 메서드: public static List<T> Repeat<T>(T value, int count)
        //               => Enumerable.Repeat(value, count).ToList();
        var repeatMethod = SyntaxFactory.MethodDeclaration(
                SyntaxFactory.GenericName("List")
                    .AddTypeArgumentListArguments(SyntaxFactory.IdentifierName("T")),
                "Repeat")
            .AddModifiers(
                SyntaxFactory.Token(SyntaxKind.PublicKeyword),
                SyntaxFactory.Token(SyntaxKind.StaticKeyword))
            .AddTypeParameterListParameters(SyntaxFactory.TypeParameter("T"))
            .AddParameterListParameters(
                Param("T",   "value"),
                Param("int", "count"))
            .WithExpressionBody(SyntaxFactory.ArrowExpressionClause(
                SyntaxFactory.ParseExpression("Enumerable.Repeat(value, count).ToList()")))
            .WithSemicolonToken(SyntaxFactory.Token(SyntaxKind.SemicolonToken));

        var helperClass = SyntaxFactory.ClassDeclaration("CollectionHelper")
            .AddModifiers(
                SyntaxFactory.Token(SyntaxKind.PublicKeyword),
                SyntaxFactory.Token(SyntaxKind.StaticKeyword))
            .AddMembers(repeatMethod);

        var nsDecl = SyntaxFactory.NamespaceDeclaration(SyntaxFactory.ParseName("Generated"))
            .AddMembers(helperClass);

        var cu = SyntaxFactory.CompilationUnit()
            .AddUsings(usings)
            .AddMembers(nsDecl)
            .NormalizeWhitespace();

        string source = cu.ToFullString();
        PrintSource(source, maxLines: 25);

        // 재파싱하여 검증
        var reparsed = CSharpSyntaxTree.ParseText(source);
        Print.Line($"재파싱 HasErrors: {reparsed.GetRoot().ContainsDiagnostics}");
        Print.Line($"재파싱 메서드 수: {reparsed.GetRoot().DescendantNodes().OfType<MethodDeclarationSyntax>().Count()}");

        // SyntaxFactory.Token 활용: trailing/leading trivia 조작
        var pubToken = SyntaxFactory.Token(
            SyntaxFactory.TriviaList(SyntaxFactory.Comment("/* generated */"), SyntaxFactory.Space),
            SyntaxKind.PublicKeyword,
            SyntaxFactory.TriviaList(SyntaxFactory.Space));
        Print.Line($"커스텀 토큰: '{pubToken.ToFullString()}'");
    }

    // ── 헬퍼 ──────────────────────────────────────────────────────────────────
    private static AccessorDeclarationSyntax Accessor(SyntaxKind kind) =>
        SyntaxFactory.AccessorDeclaration(kind)
                     .WithSemicolonToken(SyntaxFactory.Token(SyntaxKind.SemicolonToken));

    private static ParameterSyntax Param(string type, string name) =>
        SyntaxFactory.Parameter(SyntaxFactory.Identifier(name))
                     .WithType(SyntaxFactory.ParseTypeName(type));

    private static ExpressionStatementSyntax AssignStmt(string left, string right) =>
        SyntaxFactory.ExpressionStatement(
            SyntaxFactory.AssignmentExpression(
                SyntaxKind.SimpleAssignmentExpression,
                SyntaxFactory.IdentifierName(left),
                SyntaxFactory.IdentifierName(right)));

    private static void PrintSource(string src, int maxLines = 20)
    {
        var lines = src.Split('\n');
        foreach (var line in lines.Take(maxLines))
            Console.WriteLine("    " + line.TrimEnd());
        if (lines.Length > maxLines)
            Print.Line($"  ... (총 {lines.Length}줄)");
    }
}
