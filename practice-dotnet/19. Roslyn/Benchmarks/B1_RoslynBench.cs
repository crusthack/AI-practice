using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace RoslynBench;

[ShortRunJob]
[MemoryDiagnoser]
public class B1_RoslynBench
{
    private const string SimpleCode = "class Foo { void Bar() { int x = 1 + 2; } }";
    private const string LargeCode  = SimpleCode + "\n" + SimpleCode + "\n" + SimpleCode;

    private static readonly MetadataReference[] _refs = GetRefs();
    private SyntaxTree   _tree    = null!;
    private SyntaxNode   _root    = null!;
    private CSharpCompilation _comp = null!;

    [GlobalSetup]
    public void Setup()
    {
        _tree = CSharpSyntaxTree.ParseText(SimpleCode);
        _root = _tree.GetRoot();
        _comp = CSharpCompilation.Create("Bench", [_tree], _refs,
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));
    }

    // 파싱
    [Benchmark(Baseline = true)] public SyntaxTree ParseSimple()  => CSharpSyntaxTree.ParseText(SimpleCode);
    [Benchmark]                  public SyntaxTree ParseLarge()   => CSharpSyntaxTree.ParseText(LargeCode);

    // 탐색
    [Benchmark] public int WalkDescendantNodes()
        => _root.DescendantNodes().Count();

    [Benchmark] public int WalkOfType()
        => _root.DescendantNodes().OfType<MethodDeclarationSyntax>().Count();

    // Trivia
    [Benchmark] public int WalkTrivia()
        => _root.DescendantTrivia().Count();

    // Rewriter
    [Benchmark] public SyntaxNode NormalizeWhitespace()
        => _root.NormalizeWhitespace();

    // Semantic
    [Benchmark] public SemanticModel GetSemanticModel()
        => _comp.GetSemanticModel(_tree);

    [Benchmark] public ImmutableArray<Diagnostic> GetDiagnostics()
        => _comp.GetDiagnostics();

    // SyntaxFactory
    [Benchmark] public SyntaxNode BuildClass()
    {
        var cls = SyntaxFactory.ClassDeclaration("Generated")
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword));
        return SyntaxFactory.CompilationUnit().AddMembers(cls);
    }

    private static MetadataReference[] GetRefs()
    {
        var trusted = ((string?)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES"))
            ?.Split(Path.PathSeparator) ?? [];
        var needed = new System.Collections.Generic.HashSet<string>(StringComparer.OrdinalIgnoreCase)
            { "System.Private.CoreLib.dll", "System.Runtime.dll" };
        return trusted.Where(p => needed.Contains(System.IO.Path.GetFileName(p)))
                      .Select(p => (MetadataReference)MetadataReference.CreateFromFile(p))
                      .ToArray();
    }
}
