using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Diagnostics;
using System.Collections.Immutable;

namespace Roslyn.Demos;

static class D3_Analysis
{
    private const string ProblematicCode = """
        using System;

        namespace Service
        {
            public class OrderService
            {
                private string apiKey = "SECRET_KEY_12345";  // 하드코딩된 비밀

                public void ProcessOrder(int orderId)
                {
                    // TODO: 유효성 검사 추가
                    try
                    {
                        var result = CallApi(orderId);
                        Console.WriteLine(result);
                    }
                    catch (Exception)
                    {
                        // 빈 catch — EMPTYCAT001
                    }
                }

                private string CallApi(int id) => $"order-{id}";

                public void LongMethod()
                {
                    int a = 1; int b = 2; int c = 3; int d = 4;
                    int e = a + b; int f = c + d;
                    int g = e + f; int h = g * 2;
                    int i = h - 1; int j = i / 2;
                    Console.WriteLine(j);
                }

                // 명명 위반: 'M'은 너무 짧음
                public void M() { }

                public string badlyNamedProperty { get; set; } = "";
            }
        }
        """;

    private static MetadataReference[] GetBasicRefs()
    {
        var trusted = ((string?)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES"))
            ?.Split(Path.PathSeparator) ?? [];
        var needed = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            { "System.Private.CoreLib.dll", "System.Runtime.dll",
              "System.Console.dll", "netstandard.dll" };
        return trusted.Where(p => needed.Contains(Path.GetFileName(p)))
                      .Select(p => (MetadataReference)MetadataReference.CreateFromFile(p))
                      .ToArray();
    }

    public static void Run()
    {
        Print.Header("3. Analysis — 커스텀 규칙 · DiagnosticAnalyzer 프로그래매틱 실행");
        ShowCustomRules();
        ShowDiagnosticAnalyzerAPI();
        ShowCompilerDiagnostics();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowCustomRules()
    {
        Print.Section("3-1. 커스텀 규칙 (Walker 기반 정적 분석)");

        var tree = CSharpSyntaxTree.ParseText(ProblematicCode);
        var root = (CompilationUnitSyntax)tree.GetRoot();

        // 규칙 1: 빈 catch 블록
        var catchWalker = new EmptyCatchWalker2(tree);
        catchWalker.Visit(root);
        Print.Line($"[EMPTY_CATCH] 빈 catch 블록: {catchWalker.Lines.Count}개");
        foreach (int ln in catchWalker.Lines)
            Print.Line($"  → Line {ln}");

        // 규칙 2: TODO 주석
        var todos = root.DescendantTrivia()
            .Where(t => t.IsKind(SyntaxKind.SingleLineCommentTrivia) &&
                        t.ToString().Contains("TODO", StringComparison.OrdinalIgnoreCase))
            .ToList();
        Print.Line($"[TODO] 미처리 TODO: {todos.Count}개");
        foreach (var t in todos)
        {
            var pos = tree.GetLineSpan(t.Span).StartLinePosition;
            Print.Line($"  → Line {pos.Line + 1}: {t.ToString().Trim()}");
        }

        // 규칙 3: 이름 너무 짧은 메서드 (≤2자)
        var shortNameWalker = new ShortNameWalker(tree, minLength: 3);
        shortNameWalker.Visit(root);
        Print.Line($"[SHORT_NAME] 이름 너무 짧은 메서드: {shortNameWalker.Violations.Count}개");
        foreach (var (name, line) in shortNameWalker.Violations)
            Print.Line($"  → '{name}' @ Line {line}");

        // 규칙 4: 프로퍼티 명명 위반 (PascalCase가 아님)
        var propWalker = new NamingViolationWalker(tree);
        propWalker.Visit(root);
        Print.Line($"[NAMING] 프로퍼티 명명 위반: {propWalker.Violations.Count}개");
        foreach (var (name, line) in propWalker.Violations)
            Print.Line($"  → '{name}' @ Line {line}");

        // 규칙 5: 하드코딩된 시크릿 패턴
        var secretWalker = new HardcodedSecretWalker(tree);
        secretWalker.Visit(root);
        Print.Line($"[SECRET] 하드코딩된 시크릿 의심: {secretWalker.Suspects.Count}개");
        foreach (var (val, line) in secretWalker.Suspects)
            Print.Line($"  → \"{val}\" @ Line {line}");

        // 규칙 6: 메서드 복잡도 (문장 수 기준)
        var complexityWalker = new MethodComplexityWalker(threshold: 5);
        complexityWalker.Visit(root);
        Print.Line($"[COMPLEXITY] 복잡 메서드(>{5}문장): {complexityWalker.Complex.Count}개");
        foreach (var (name, cnt) in complexityWalker.Complex)
            Print.Line($"  → {name}: {cnt}개 문장");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowDiagnosticAnalyzerAPI()
    {
        Print.Section("3-2. DiagnosticAnalyzer 프로그래매틱 실행");

        var tree = CSharpSyntaxTree.ParseText(ProblematicCode);
        var comp = CSharpCompilation.Create("Demo",
            syntaxTrees: [tree],
            references: GetBasicRefs(),
            options: new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));

        // DiagnosticAnalyzer 인스턴스를 직접 만들어 CompilationWithAnalyzers 실행
        var analyzers  = ImmutableArray.Create<DiagnosticAnalyzer>(new DemoEmptyCatchAnalyzer());
        var withAnalyzers = comp.WithAnalyzers(analyzers);
        var diagnostics = withAnalyzers.GetAnalyzerDiagnosticsAsync().GetAwaiter().GetResult();

        Print.Line($"DiagnosticAnalyzer 결과: {diagnostics.Length}개");
        foreach (var d in diagnostics)
        {
            var pos = d.Location.GetLineSpan().StartLinePosition;
            Print.Line($"  [{d.Id}] {d.Severity} {d.GetMessage()}  @ Line {pos.Line + 1}");
        }

        // 컴파일러 진단 + 분석기 진단 합산
        var allDiags = withAnalyzers.GetAllDiagnosticsAsync().GetAwaiter().GetResult();
        var bySeverity = allDiags.GroupBy(d => d.Severity).ToDictionary(g => g.Key, g => g.Count());
        foreach (var (sev, cnt) in bySeverity)
            Print.Line($"  {sev}: {cnt}개");

        // DiagnosticDescriptor 정보 출력
        var analyzer = analyzers[0];
        Print.Line($"분석기: {analyzer.GetType().Name}");
        foreach (var desc in analyzer.SupportedDiagnostics)
        {
            Print.Line($"  ID={desc.Id}  Title={desc.Title}");
            Print.Line($"  Category={desc.Category}  Severity={desc.DefaultSeverity}  Enabled={desc.IsEnabledByDefault}");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowCompilerDiagnostics()
    {
        Print.Section("3-3. 컴파일러 진단 — 경고 레벨 · Suppression");

        // nullable 비활성 코드
        const string nullable = """
            #nullable enable
            class Foo {
                string? name = null;
                void Bar() { int len = name.Length; }  // CS8602: dereference of null
            }
            """;

        var tree = CSharpSyntaxTree.ParseText(nullable,
            CSharpParseOptions.Default.WithLanguageVersion(LanguageVersion.Latest));
        var comp = CSharpCompilation.Create("Nullable", [tree], GetBasicRefs(),
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary,
                nullableContextOptions: NullableContextOptions.Enable));

        var diags = comp.GetDiagnostics();
        Print.Line($"Nullable 분석 진단: {diags.Length}개");
        foreach (var d in diags.Where(d => d.Severity >= DiagnosticSeverity.Warning))
        {
            var pos = d.Location.GetLineSpan().StartLinePosition;
            Print.Line($"  [{d.Id}] {d.GetMessage()}  @ Line {pos.Line + 1}");
        }

        // ParseOptions — 언어 버전 확인
        var parseOpts = (CSharpParseOptions)tree.Options;
        Print.Line($"언어 버전: {parseOpts.LanguageVersion}  SpecifiedVersion: {parseOpts.SpecifiedLanguageVersion}");

        // CompilationOptions 정보
        var compOpts = (CSharpCompilationOptions)comp.Options;
        Print.Line($"OutputKind: {compOpts.OutputKind}  OptimizationLevel: {compOpts.OptimizationLevel}");
        Print.Line($"Nullable: {compOpts.NullableContextOptions}  Platform: {compOpts.Platform}");
    }
}

// ── Custom Walkers ────────────────────────────────────────────────────────────

sealed class EmptyCatchWalker2(SyntaxTree tree) : CSharpSyntaxWalker
{
    public List<int> Lines { get; } = new();
    public override void VisitCatchClause(CatchClauseSyntax node)
    {
        if (node.Block.Statements.Count == 0)
            Lines.Add(tree.GetLineSpan(node.Span).StartLinePosition.Line + 1);
        base.VisitCatchClause(node);
    }
}

sealed class ShortNameWalker(SyntaxTree tree, int minLength) : CSharpSyntaxWalker
{
    public List<(string Name, int Line)> Violations { get; } = new();
    public override void VisitMethodDeclaration(MethodDeclarationSyntax node)
    {
        if (node.Identifier.Text.Length < minLength)
        {
            int line = tree.GetLineSpan(node.Span).StartLinePosition.Line + 1;
            Violations.Add((node.Identifier.Text, line));
        }
        base.VisitMethodDeclaration(node);
    }
}

sealed class NamingViolationWalker(SyntaxTree tree) : CSharpSyntaxWalker
{
    public List<(string Name, int Line)> Violations { get; } = new();
    public override void VisitPropertyDeclaration(PropertyDeclarationSyntax node)
    {
        string name = node.Identifier.Text;
        if (name.Length > 0 && char.IsLower(name[0]))
        {
            int line = tree.GetLineSpan(node.Span).StartLinePosition.Line + 1;
            Violations.Add((name, line));
        }
        base.VisitPropertyDeclaration(node);
    }
}

sealed class HardcodedSecretWalker(SyntaxTree tree) : CSharpSyntaxWalker
{
    private static readonly string[] _keywords = ["secret", "key", "password", "token", "pwd"];
    public List<(string Value, int Line)> Suspects { get; } = new();

    public override void VisitVariableDeclarator(VariableDeclaratorSyntax node)
    {
        string varName = node.Identifier.Text.ToLowerInvariant();
        if (_keywords.Any(k => varName.Contains(k)) &&
            node.Initializer?.Value is LiteralExpressionSyntax lit &&
            lit.IsKind(SyntaxKind.StringLiteralExpression))
        {
            string val  = lit.Token.ValueText;
            int line    = tree.GetLineSpan(node.Span).StartLinePosition.Line + 1;
            Suspects.Add((val.Length > 10 ? val[..10] + "…" : val, line));
        }
        base.VisitVariableDeclarator(node);
    }
}

sealed class MethodComplexityWalker(int threshold) : CSharpSyntaxWalker
{
    public List<(string Name, int StatementCount)> Complex { get; } = new();
    public override void VisitMethodDeclaration(MethodDeclarationSyntax node)
    {
        int count = node.DescendantNodes().OfType<StatementSyntax>().Count();
        if (count > threshold)
            Complex.Add((node.Identifier.Text, count));
        base.VisitMethodDeclaration(node);
    }
}

// ── Inline DiagnosticAnalyzer (Demo용, 실제는 Analyzer/ 프로젝트 참고) ──────────

[DiagnosticAnalyzer(LanguageNames.CSharp)]
sealed class DemoEmptyCatchAnalyzer : DiagnosticAnalyzer
{
    public const string DiagnosticId = "EMPTYCAT001";

    private static readonly DiagnosticDescriptor _rule = new(
        id: DiagnosticId,
        title: "빈 catch 블록",
        messageFormat: "예외를 무시하는 빈 catch 블록: 최소 로그 또는 throw를 추가하세요",
        category: "BestPractice",
        defaultSeverity: DiagnosticSeverity.Warning,
        isEnabledByDefault: true,
        description: "빈 catch 블록은 예외를 무음으로 삼킵니다.");

    public override ImmutableArray<DiagnosticDescriptor> SupportedDiagnostics => [_rule];

    public override void Initialize(AnalysisContext context)
    {
        context.ConfigureGeneratedCodeAnalysis(GeneratedCodeAnalysisFlags.None);
        context.EnableConcurrentExecution();
        context.RegisterSyntaxNodeAction(Analyze, SyntaxKind.CatchClause);
    }

    private static void Analyze(SyntaxNodeAnalysisContext ctx)
    {
        var clause = (CatchClauseSyntax)ctx.Node;
        if (clause.Block.Statements.Count == 0)
            ctx.ReportDiagnostic(Diagnostic.Create(_rule, clause.GetLocation()));
    }
}
