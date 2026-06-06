using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Diagnostics;
using System.Collections.Immutable;
using Xunit;

namespace Roslyn.Tests;

public class SyntaxTreeTests
{
    private const string SimpleCode = """
        namespace App {
            public class Foo {
                public int Add(int a, int b) => a + b;
                public void Bar() { try { } catch (Exception) { } }
                public string Baz { get; set; } = "";
            }
        }
        """;

    [Fact] public void ParseText_NoErrors()
    {
        var root = CSharpSyntaxTree.ParseText(SimpleCode).GetRoot();
        Assert.False(root.ContainsDiagnostics);
    }

    [Fact] public void DescendantNodes_CountsMethods()
    {
        var root    = CSharpSyntaxTree.ParseText(SimpleCode).GetRoot();
        var methods = root.DescendantNodes().OfType<MethodDeclarationSyntax>().ToList();
        Assert.Equal(2, methods.Count);
    }

    [Fact] public void DescendantNodes_CountsProperties()
    {
        var root  = CSharpSyntaxTree.ParseText(SimpleCode).GetRoot();
        var props = root.DescendantNodes().OfType<PropertyDeclarationSyntax>().ToList();
        Assert.Single(props);
    }

    [Fact] public void SyntaxWalker_CollectsIdentifiers()
    {
        var root   = CSharpSyntaxTree.ParseText(SimpleCode).GetRoot();
        var walker = new IdentifierWalker();
        walker.Visit(root);
        // IdentifierNameSyntax: 표현식의 이름 참조 수집 (선언 토큰 제외)
        Assert.Contains("a", walker.Names);          // Add 메서드 파라미터 a 참조
        Assert.Contains("Exception", walker.Names);  // catch (Exception)
        Assert.NotEmpty(walker.Names);
    }

    [Fact] public void EmptyCatch_Detection()
    {
        var tree      = CSharpSyntaxTree.ParseText(SimpleCode);
        var walker    = new EmptyCatchTestWalker(tree);
        walker.Visit(tree.GetRoot());
        Assert.Single(walker.EmptyLines);
    }

    [Fact] public void SyntaxRewriter_RenamesMethod()
    {
        var tree    = CSharpSyntaxTree.ParseText(SimpleCode);
        var root    = (CompilationUnitSyntax)tree.GetRoot();
        var rewriter = new PrefixRewriter("Test_");
        var newRoot  = rewriter.Visit(root);
        var names    = newRoot.DescendantNodes().OfType<MethodDeclarationSyntax>()
                              .Select(m => m.Identifier.Text).ToList();
        Assert.All(names, n => Assert.StartsWith("Test_", n));
    }

    [Fact] public void NormalizeWhitespace_RemovesUglySpacing()
    {
        const string ugly   = "class Foo{void Bar(){}}";
        var root            = CSharpSyntaxTree.ParseText(ugly).GetRoot();
        var normalized      = root.NormalizeWhitespace().ToFullString();
        Assert.Contains("void Bar()", normalized);
        Assert.True(normalized.Length > ugly.Length);
    }

    [Fact] public void TriviaAnalysis_FindsTODO()
    {
        const string code = "class Foo { void M() { /* TODO: fix */ } }";
        var tree  = CSharpSyntaxTree.ParseText(code);
        var todos = tree.GetRoot().DescendantTrivia()
                        .Where(t => t.ToString().Contains("TODO")).ToList();
        Assert.Single(todos);
    }

    [Fact] public void GetLineSpan_ReturnsCorrectLine()
    {
        var tree   = CSharpSyntaxTree.ParseText(SimpleCode);
        var method = tree.GetRoot().DescendantNodes().OfType<MethodDeclarationSyntax>().First();
        var span   = tree.GetLineSpan(method.Span);
        Assert.True(span.StartLinePosition.Line >= 0);
    }

    [Fact] public void SyntaxAnnotation_RoundTrip()
    {
        var root  = CSharpSyntaxTree.ParseText(SimpleCode).GetRoot();
        var m     = root.DescendantNodes().OfType<MethodDeclarationSyntax>().First();
        var ann   = new SyntaxAnnotation("tag", "value");
        var newRoot = root.ReplaceNode(m, m.WithAdditionalAnnotations(ann));
        var found   = newRoot.GetAnnotatedNodes("tag").FirstOrDefault();
        Assert.NotNull(found);
        Assert.Equal("value", found.GetAnnotations("tag").First().Data);
    }
}

public class SemanticModelTests
{
    private static MetadataReference[] GetRefs()
    {
        var trusted = ((string?)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES"))
            ?.Split(Path.PathSeparator) ?? [];
        var needed = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            { "System.Private.CoreLib.dll", "System.Runtime.dll" };
        return trusted.Where(p => needed.Contains(Path.GetFileName(p)))
                      .Select(p => (MetadataReference)MetadataReference.CreateFromFile(p))
                      .ToArray();
    }

    private const string TypeCode = """
        namespace T {
            public interface IFoo { void Run(); }
            public class Bar : IFoo { public void Run() { } }
        }
        """;

    [Fact] public void GetDeclaredSymbol_ReturnsNamedType()
    {
        var tree  = CSharpSyntaxTree.ParseText(TypeCode);
        var comp  = CSharpCompilation.Create("T", [tree], GetRefs(),
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));
        var model = comp.GetSemanticModel(tree);
        var cls   = tree.GetRoot().DescendantNodes().OfType<ClassDeclarationSyntax>().First();
        var sym   = model.GetDeclaredSymbol(cls) as INamedTypeSymbol;
        Assert.NotNull(sym);
        Assert.Equal("Bar", sym.Name);
        Assert.Equal(TypeKind.Class, sym.TypeKind);
    }

    [Fact] public void InterfaceImplementation_IsDetected()
    {
        var tree  = CSharpSyntaxTree.ParseText(TypeCode);
        var comp  = CSharpCompilation.Create("T", [tree], GetRefs(),
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));
        var model = comp.GetSemanticModel(tree);
        var root  = tree.GetRoot();

        var ifaceDecl = root.DescendantNodes().OfType<InterfaceDeclarationSyntax>().First();
        var clsDecl   = root.DescendantNodes().OfType<ClassDeclarationSyntax>().First();

        var ifaceSym = (INamedTypeSymbol)model.GetDeclaredSymbol(ifaceDecl)!;
        var clsSym   = (INamedTypeSymbol)model.GetDeclaredSymbol(clsDecl)!;

        Assert.Contains(clsSym.AllInterfaces, i =>
            SymbolEqualityComparer.Default.Equals(i, ifaceSym));
    }

    [Fact] public void Compilation_NoErrors_ForValidCode()
    {
        var tree = CSharpSyntaxTree.ParseText(TypeCode);
        var comp = CSharpCompilation.Create("T", [tree], GetRefs(),
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));
        var errors = comp.GetDiagnostics().Where(d => d.Severity == DiagnosticSeverity.Error);
        Assert.Empty(errors);
    }

    [Fact] public void DataFlowAnalysis_AlwaysAssigned()
    {
        const string code = "class C { int M(int a, int b) { int r = a + b; return r; } }";
        var tree  = CSharpSyntaxTree.ParseText(code);
        var comp  = CSharpCompilation.Create("C", [tree], GetRefs(),
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));
        var model = comp.GetSemanticModel(tree);
        var body  = tree.GetRoot().DescendantNodes().OfType<MethodDeclarationSyntax>()
                        .First().Body!;
        var df    = model.AnalyzeDataFlow(body)!;
        Assert.Contains(df.AlwaysAssigned, s => s.Name == "r");
    }

    [Fact] public void ControlFlowAnalysis_EndpointReachable()
    {
        const string code = "class C { int M() { int x = 1; return x; } }";
        var tree  = CSharpSyntaxTree.ParseText(code);
        var comp  = CSharpCompilation.Create("C", [tree], GetRefs(),
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));
        var model = comp.GetSemanticModel(tree);
        var body  = tree.GetRoot().DescendantNodes().OfType<MethodDeclarationSyntax>()
                        .First().Body!;
        var cf = model.AnalyzeControlFlow(body)!;
        Assert.False(cf.EndPointIsReachable);  // return으로 끝나므로
    }
}

public class DiagnosticAnalyzerTests
{
    private static MetadataReference[] GetRefs()
    {
        var trusted = ((string?)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES"))
            ?.Split(Path.PathSeparator) ?? [];
        var needed = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            { "System.Private.CoreLib.dll", "System.Runtime.dll" };
        return trusted.Where(p => needed.Contains(Path.GetFileName(p)))
                      .Select(p => (MetadataReference)MetadataReference.CreateFromFile(p))
                      .ToArray();
    }

    [Fact] public async Task EmptyCatchAnalyzer_ReportsOnEmptyCatch()
    {
        const string code = "class C { void M() { try { } catch (System.Exception) { } } }";
        var tree = CSharpSyntaxTree.ParseText(code);
        var comp = CSharpCompilation.Create("C", [tree], GetRefs(),
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));

        var analyzers = ImmutableArray.Create<DiagnosticAnalyzer>(new DemoEmptyCatch());
        var diags = await comp.WithAnalyzers(analyzers).GetAnalyzerDiagnosticsAsync();
        Assert.Single(diags);
        Assert.Equal("EMPTYCAT001", diags[0].Id);
    }

    [Fact] public async Task EmptyCatchAnalyzer_NoReportOnNonEmptyCatch()
    {
        const string code = "class C { void M() { try { } catch (System.Exception) { throw; } } }";
        var tree = CSharpSyntaxTree.ParseText(code);
        var comp = CSharpCompilation.Create("C", [tree], GetRefs(),
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));

        var analyzers = ImmutableArray.Create<DiagnosticAnalyzer>(new DemoEmptyCatch());
        var diags = await comp.WithAnalyzers(analyzers).GetAnalyzerDiagnosticsAsync();
        Assert.Empty(diags);
    }
}

public class SyntaxFactoryTests
{
    [Fact] public void ParseExpression_ReturnsExpressionSyntax()
    {
        var expr = SyntaxFactory.ParseExpression("1 + 2");
        Assert.NotNull(expr);
        Assert.IsType<BinaryExpressionSyntax>(expr);
    }

    [Fact] public void BuildClass_CanBeReparsed()
    {
        var cls = SyntaxFactory.ClassDeclaration("TestClass")
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword));
        var cu  = SyntaxFactory.CompilationUnit().AddMembers(cls).NormalizeWhitespace();
        var reparsed = CSharpSyntaxTree.ParseText(cu.ToFullString()).GetRoot();
        Assert.False(reparsed.ContainsDiagnostics);
        Assert.Single(reparsed.DescendantNodes().OfType<ClassDeclarationSyntax>());
    }

    [Fact] public void BuildRecord_SyntaxKindIsRecord()
    {
        var rec = SyntaxFactory.RecordDeclaration(
                SyntaxFactory.Token(SyntaxKind.RecordKeyword), "Pt")
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword))
            .WithSemicolonToken(SyntaxFactory.Token(SyntaxKind.SemicolonToken));
        Assert.True(rec.IsKind(SyntaxKind.RecordDeclaration));
    }
}

// ── Test 전용 헬퍼 Walker/Rewriter ────────────────────────────────────────────

sealed class IdentifierWalker : CSharpSyntaxWalker
{
    public HashSet<string> Names { get; } = new();
    public override void VisitIdentifierName(IdentifierNameSyntax node)
    {
        Names.Add(node.Identifier.Text);
        base.VisitIdentifierName(node);
    }
}

sealed class EmptyCatchTestWalker(SyntaxTree tree) : CSharpSyntaxWalker
{
    public List<int> EmptyLines { get; } = new();
    public override void VisitCatchClause(CatchClauseSyntax node)
    {
        if (node.Block.Statements.Count == 0)
            EmptyLines.Add(tree.GetLineSpan(node.Span).StartLinePosition.Line + 1);
        base.VisitCatchClause(node);
    }
}

sealed class PrefixRewriter(string prefix) : CSharpSyntaxRewriter
{
    public override SyntaxNode? VisitMethodDeclaration(MethodDeclarationSyntax node)
    {
        var newId = SyntaxFactory.Identifier(prefix + node.Identifier.Text)
                                 .WithTriviaFrom(node.Identifier);
        return base.VisitMethodDeclaration(node.WithIdentifier(newId));
    }
}

[DiagnosticAnalyzer(LanguageNames.CSharp)]
sealed class DemoEmptyCatch : DiagnosticAnalyzer
{
    private static readonly DiagnosticDescriptor Rule = new(
        "EMPTYCAT001", "빈 catch", "빈 catch", "Test",
        DiagnosticSeverity.Warning, true);

    public override ImmutableArray<DiagnosticDescriptor> SupportedDiagnostics => [Rule];

    public override void Initialize(AnalysisContext context)
    {
        context.ConfigureGeneratedCodeAnalysis(GeneratedCodeAnalysisFlags.None);
        context.EnableConcurrentExecution();
        context.RegisterSyntaxNodeAction(ctx =>
        {
            var clause = (CatchClauseSyntax)ctx.Node;
            if (clause.Block.Statements.Count == 0)
                ctx.ReportDiagnostic(Diagnostic.Create(Rule, clause.GetLocation()));
        }, SyntaxKind.CatchClause);
    }
}
