using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace Roslyn.Demos;

static class D2_SemanticModel
{
    private const string ShapeCode = """
        using System;

        namespace Shapes
        {
            public interface IShape
            {
                double Area();
                double Perimeter();
            }

            public class Circle : IShape
            {
                public double Radius { get; init; }

                public Circle(double radius) { Radius = radius; }

                public double Area()      => Math.PI * Radius * Radius;
                public double Perimeter() => 2 * Math.PI * Radius;
            }

            public class Rectangle : IShape
            {
                public double Width  { get; init; }
                public double Height { get; init; }

                public Rectangle(double w, double h) { Width = w; Height = h; }

                public double Area()      => Width * Height;
                public double Perimeter() => 2 * (Width + Height);
            }
        }
        """;

    private static MetadataReference[] GetBasicRefs()
    {
        var trusted = ((string?)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES"))
            ?.Split(Path.PathSeparator) ?? [];
        var needed = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            { "System.Private.CoreLib.dll", "System.Runtime.dll", "netstandard.dll" };
        return trusted.Where(p => needed.Contains(Path.GetFileName(p)))
                      .Select(p => (MetadataReference)MetadataReference.CreateFromFile(p))
                      .ToArray();
    }

    private static (CSharpCompilation comp, SyntaxTree tree) MakeCompilation(string src, string name = "Demo")
    {
        var tree = CSharpSyntaxTree.ParseText(src);
        var comp = CSharpCompilation.Create(name,
            syntaxTrees: [tree],
            references: GetBasicRefs(),
            options: new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));
        return (comp, tree);
    }

    public static void Run()
    {
        Print.Header("2. Semantic Model — 의미 분석 · 심볼 · 타입");
        ShowCompilationDiagnostics();
        ShowDeclaredSymbols();
        ShowTypeHierarchy();
        ShowSymbolInfo();
        ShowFlowAnalysis();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowCompilationDiagnostics()
    {
        Print.Section("2-1. Compilation 생성 & 진단");

        var (comp, _) = MakeCompilation(ShapeCode, "Shapes");
        var diags  = comp.GetDiagnostics();
        var errors = diags.Where(d => d.Severity == DiagnosticSeverity.Error).ToList();
        var warns  = diags.Where(d => d.Severity == DiagnosticSeverity.Warning).ToList();

        Print.Line($"어셈블리:  {comp.AssemblyName}");
        Print.Line($"SyntaxTree: {comp.SyntaxTrees.Length}개  참조: {comp.References.Count()}개");
        Print.Line($"오류: {errors.Count}  경고: {warns.Count}");
        foreach (var e in errors.Take(3))
            Print.Line($"  ERROR [{e.Id}] {e.GetMessage()}");

        // 오류가 있는 코드로 테스트
        const string bad = "class Foo { void Bar() { int x = \"string\"; } }";
        var (badComp, _) = MakeCompilation(bad, "Bad");
        var badErrors = badComp.GetDiagnostics()
                               .Where(d => d.Severity == DiagnosticSeverity.Error).ToList();
        Print.Line($"의도적 오류 코드 → 오류 {badErrors.Count}개:");
        foreach (var e in badErrors.Take(3))
        {
            var pos = e.Location.GetLineSpan().StartLinePosition;
            Print.Line($"  [{e.Id}] {e.GetMessage()}  @ Line {pos.Line + 1}:{pos.Character}");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowDeclaredSymbols()
    {
        Print.Section("2-2. 선언 심볼 — INamedTypeSymbol · IMethodSymbol · IPropertySymbol");

        var (comp, tree) = MakeCompilation(ShapeCode);
        var model = comp.GetSemanticModel(tree);
        var root  = (CompilationUnitSyntax)tree.GetRoot();

        foreach (var typeDecl in root.DescendantNodes().OfType<TypeDeclarationSyntax>())
        {
            var sym = (INamedTypeSymbol?)model.GetDeclaredSymbol(typeDecl);
            if (sym is null) continue;

            Print.Line($"[{sym.TypeKind}] {sym.ToDisplayString()}");
            Print.Line($"  IsAbstract={sym.IsAbstract}  IsSealed={sym.IsSealed}  Arity={sym.Arity}");
            Print.Line($"  Interfaces: [{string.Join(", ", sym.Interfaces.Select(i => i.Name))}]");
            Print.Line($"  BaseType: {sym.BaseType?.ToDisplayString()}");

            foreach (var m in sym.GetMembers().OfType<IMethodSymbol>()
                                 .Where(m => m.MethodKind == MethodKind.Ordinary))
            {
                string @params = string.Join(", ", m.Parameters.Select(p => $"{p.Type.Name} {p.Name}"));
                Print.Line($"  method: {m.ReturnType.Name} {m.Name}({@params})  IsVirtual={m.IsVirtual}  IsOverride={m.IsOverride}");
            }

            foreach (var p in sym.GetMembers().OfType<IPropertySymbol>())
                Print.Line($"  prop:   {p.Type.Name} {p.Name}  IsReadOnly={p.IsReadOnly}  HasGet={p.GetMethod is not null}  HasSet={p.SetMethod is not null}");
        }

        // SymbolDisplayFormat 변형
        var circleDecl = root.DescendantNodes().OfType<ClassDeclarationSyntax>()
                             .First(c => c.Identifier.Text == "Circle");
        var circleSym = (INamedTypeSymbol)model.GetDeclaredSymbol(circleDecl)!;
        Print.Line($"Minimal:   {circleSym.ToDisplayString(SymbolDisplayFormat.MinimallyQualifiedFormat)}");
        Print.Line($"Full:      {circleSym.ToDisplayString(SymbolDisplayFormat.FullyQualifiedFormat)}");
        Print.Line($"CSharpErr: {circleSym.ToDisplayString(SymbolDisplayFormat.CSharpErrorMessageFormat)}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowTypeHierarchy()
    {
        Print.Section("2-3. 타입 계층 · 인터페이스 구현 매핑");

        var (comp, tree) = MakeCompilation(ShapeCode);
        var model = comp.GetSemanticModel(tree);
        var root  = (CompilationUnitSyntax)tree.GetRoot();

        var ifaceDecl = root.DescendantNodes().OfType<InterfaceDeclarationSyntax>().First();
        var ifaceSym  = (INamedTypeSymbol?)model.GetDeclaredSymbol(ifaceDecl);

        foreach (var classDecl in root.DescendantNodes().OfType<ClassDeclarationSyntax>())
        {
            var cls = (INamedTypeSymbol?)model.GetDeclaredSymbol(classDecl);
            if (cls is null) continue;

            bool impl = cls.AllInterfaces.Any(i => SymbolEqualityComparer.Default.Equals(i, ifaceSym));
            Print.Line($"{cls.Name} implements IShape: {impl}");

            if (impl && ifaceSym is not null)
            {
                foreach (var member in ifaceSym.GetMembers().OfType<IMethodSymbol>())
                {
                    var implSym = cls.FindImplementationForInterfaceMember(member);
                    Print.Line($"  IShape.{member.Name}() → {implSym?.ContainingType.Name}.{implSym?.Name}");
                }
            }
        }

        // 소스 코드 정의 네임스페이스 트리만 출력 (BCL 제외)
        var globalNs     = comp.GlobalNamespace;
        var sourceNs = globalNs.GetNamespaceMembers()
            .Where(n => n.GetNamespaceMembers().Any() ||
                        n.GetTypeMembers().Any())
            .Where(n => n.Name is not ("System" or "Microsoft" or "FxResources" or "Internal"));
        Print.Line("네임스페이스 트리 (소스 정의):");
        foreach (var ns in sourceNs)
            PrintNamespace(ns, 1);
    }

    static void PrintNamespace(INamespaceSymbol ns, int depth)
    {
        if (depth > 3) return;
        string indent = new string(' ', depth * 2 + 2);
        Console.WriteLine($"    {indent}namespace {ns.Name}");
        foreach (var type in ns.GetTypeMembers())
            Console.WriteLine($"    {indent}  [{type.TypeKind}] {type.Name}");
        foreach (var child in ns.GetNamespaceMembers())
            PrintNamespace(child, depth + 1);
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowSymbolInfo()
    {
        Print.Section("2-4. GetSymbolInfo & GetTypeInfo — 표현식 심볼 조회");

        const string exprCode = """
            using System;
            using System.Collections.Generic;
            class Demo {
                void Run() {
                    double r = 5.0;
                    double area = Math.PI * r * r;
                    var list = new List<int>();
                    list.Add(42);
                    Console.WriteLine(area);
                }
            }
            """;

        var trusted = ((string?)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES"))
            ?.Split(Path.PathSeparator) ?? [];
        var needed = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
        {
            "System.Private.CoreLib.dll", "System.Runtime.dll",
            "System.Console.dll", "System.Collections.dll"
        };
        var refs = trusted.Where(p => needed.Contains(Path.GetFileName(p)))
                          .Select(p => (MetadataReference)MetadataReference.CreateFromFile(p))
                          .ToArray();

        var tree  = CSharpSyntaxTree.ParseText(exprCode);
        var comp  = CSharpCompilation.Create("Expr", [tree], refs,
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));
        var model = comp.GetSemanticModel(tree);
        var root  = tree.GetRoot();

        // 로컬 변수 심볼
        Print.Line("로컬 변수:");
        foreach (var decl in root.DescendantNodes().OfType<LocalDeclarationStatementSyntax>())
            foreach (var v in decl.Declaration.Variables)
            {
                var sym = (ILocalSymbol?)model.GetDeclaredSymbol(v);
                if (sym is not null)
                    Print.Line($"  {sym.Type.ToDisplayString()} {sym.Name}  IsConst={sym.IsConst}");
            }

        // 메서드 호출 심볼
        Print.Line("메서드 호출:");
        foreach (var inv in root.DescendantNodes().OfType<InvocationExpressionSyntax>())
        {
            var info = model.GetSymbolInfo(inv);
            if (info.Symbol is IMethodSymbol m)
                Print.Line($"  {m.ContainingType.Name}.{m.Name}()  반환={m.ReturnType.ToDisplayString()}  static={m.IsStatic}");
        }

        // new 표현식 타입
        Print.Line("객체 생성:");
        foreach (var oc in root.DescendantNodes().OfType<ObjectCreationExpressionSyntax>())
        {
            var typeInfo = model.GetTypeInfo(oc);
            Print.Line($"  new {typeInfo.Type?.ToDisplayString()}");
        }

        // 멤버 접근 (MemberAccessExpressionSyntax)
        Print.Line("멤버 접근:");
        foreach (var ma in root.DescendantNodes().OfType<MemberAccessExpressionSyntax>().Take(4))
        {
            var info = model.GetSymbolInfo(ma);
            if (info.Symbol is { } sym)
                Print.Line($"  {ma} → {sym.Kind}: {sym.Name}");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowFlowAnalysis()
    {
        Print.Section("2-5. DataFlow · ControlFlow 분석");

        const string flowCode = """
            class Demo {
                int Compute(int a, int b, bool flag)
                {
                    int result;
                    if (flag)
                        result = a + b;
                    else
                        result = a * b;
                    int extra = result + 1;
                    return extra;
                }
            }
            """;

        var (comp, tree) = MakeCompilation(flowCode, "Flow");
        var model  = comp.GetSemanticModel(tree);
        var method = tree.GetRoot().DescendantNodes().OfType<MethodDeclarationSyntax>().First();
        var body   = method.Body!;

        // DataFlow
        var df = model.AnalyzeDataFlow(body)!;
        Print.Line($"DataFlow.ReadInside:      [{string.Join(", ", df.ReadInside.Select(s => s.Name))}]");
        Print.Line($"DataFlow.WrittenInside:   [{string.Join(", ", df.WrittenInside.Select(s => s.Name))}]");
        Print.Line($"DataFlow.AlwaysAssigned:  [{string.Join(", ", df.AlwaysAssigned.Select(s => s.Name))}]");
        Print.Line($"DataFlow.VariablesDeclared:[{string.Join(", ", df.VariablesDeclared.Select(s => s.Name))}]");
        Print.Line($"DataFlow.WrittenOutside:  [{string.Join(", ", df.WrittenOutside.Select(s => s.Name))}]");

        // ControlFlow
        var cf = model.AnalyzeControlFlow(body)!;
        Print.Line($"ControlFlow.Succeeded:            {cf.Succeeded}");
        Print.Line($"ControlFlow.EndPointIsReachable:  {cf.EndPointIsReachable}");
        Print.Line($"ControlFlow.ReturnStatements:     {cf.ReturnStatements.Length}개");
        Print.Line($"ControlFlow.ExitPoints:           {cf.ExitPoints.Length}개");
    }
}
