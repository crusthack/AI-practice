using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace Roslyn.Demos;

static class D1_SyntaxTree
{
    private const string SampleCode = """
        using System;
        using System.Collections.Generic;

        namespace SampleApp
        {
            /// <summary>계산기 클래스</summary>
            public class Calculator
            {
                private int _history = 0;

                public int Add(int a, int b)
                {
                    _history++;
                    return a + b;
                }

                public int Subtract(int a, int b) => a - b;

                // TODO: 나눗셈 구현 필요
                public double Divide(int a, int b)
                {
                    try
                    {
                        return (double)a / b;
                    }
                    catch (DivideByZeroException)
                    {
                        // 빈 catch — 안티패턴
                    }
                    return 0;
                }

                public int History => _history;
            }
        }
        """;

    public static void Run()
    {
        Print.Header("1. Syntax Tree — 구문 분석 · 순회 · 변환");
        ShowTreeStructure();
        ShowSyntaxWalker();
        ShowSyntaxRewriter();
        ShowTriviaAnalysis();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowTreeStructure()
    {
        Print.Section("1-1. SyntaxTree 파싱 & 노드 탐색");

        SyntaxTree tree = CSharpSyntaxTree.ParseText(SampleCode);
        CompilationUnitSyntax root = tree.GetCompilationUnitRoot();

        Print.Line($"Kind:            {root.Kind()}");
        Print.Line($"ContainsDiags:   {root.ContainsDiagnostics}");
        Print.Line($"FullSpan:        [{root.FullSpan.Start}..{root.FullSpan.End}]  ({root.FullSpan.Length}자)");

        // using 목록
        var usings = root.Usings.Select(u => u.Name!.ToString());
        Print.Items(usings, "Usings: ");

        // 네임스페이스 → 클래스
        var ns  = root.DescendantNodes().OfType<NamespaceDeclarationSyntax>().First();
        var cls = ns.DescendantNodes().OfType<ClassDeclarationSyntax>().First();
        Print.Line($"Namespace: {ns.Name}   Class: {cls.Identifier}  [modifiers: {string.Join(",", cls.Modifiers)}]");

        // 메서드 상세
        var methods = cls.DescendantNodes().OfType<MethodDeclarationSyntax>().ToList();
        Print.Line($"메서드 수: {methods.Count}");
        foreach (var m in methods)
        {
            string ret    = m.ReturnType.ToString();
            string @params = string.Join(", ", m.ParameterList.Parameters.Select(p => $"{p.Type} {p.Identifier}"));
            bool isExpr   = m.ExpressionBody is not null;
            var span      = tree.GetLineSpan(m.Span);
            int lines     = span.EndLinePosition.Line - span.StartLinePosition.Line + 1;
            Print.Line($"  {ret} {m.Identifier}({@params})  expr={isExpr}  lines={lines}");
        }

        // 속성 (Property)
        var props = cls.DescendantNodes().OfType<PropertyDeclarationSyntax>().ToList();
        foreach (var p in props)
            Print.Line($"  prop: {p.Type} {p.Identifier}  accessors=[{string.Join(",", p.AccessorList?.Accessors.Select(a => a.Kind()) ?? [])}]");

        // 부모/자식 관계
        var firstMethod = methods[0];
        Print.Line($"첫 메서드의 부모 Kind: {firstMethod.Parent?.Kind()}");
        Print.Line($"메서드 Body 자식 문장: {firstMethod.Body?.Statements.Count}개");

        // FindNode (span으로 특정 노드 찾기)
        var found = root.FindNode(methods[1].Span);
        Print.Line($"FindNode 결과: {found.Kind()}  ({found.GetType().Name})");

        // SyntaxNode.Ancestors()
        var ancestors = firstMethod.Ancestors().Select(a => a.Kind().ToString()).Take(4);
        Print.Items(ancestors, "첫 메서드 조상: ");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowSyntaxWalker()
    {
        Print.Section("1-2. CSharpSyntaxWalker — 방문자 패턴");

        SyntaxTree tree = CSharpSyntaxTree.ParseText(SampleCode);
        var root = (CompilationUnitSyntax)tree.GetRoot();

        // 식별자 수집
        var idCollector = new IdentifierCollector();
        idCollector.Visit(root);
        var topIds = idCollector.Identifiers.GroupBy(x => x).OrderByDescending(g => g.Count()).Take(5);
        Print.Line($"식별자 종류: {idCollector.Identifiers.Distinct().Count()}  총 참조: {idCollector.Identifiers.Count}");
        Print.Items(topIds.Select(g => $"{g.Key}({g.Count()})"), "빈도 top5: ");

        // 메서드 분석
        var methodWalker = new MethodAnalysisWalker(tree);
        methodWalker.Visit(root);
        Print.Line("메서드 분석:");
        foreach (var info in methodWalker.Methods)
            Print.Line($"  {info.ReturnType} {info.Name}({info.ParamCount}개)  lines={info.LineCount}  expr={info.IsExpression}");

        // 빈 catch 감지
        var catchWalker = new EmptyCatchWalker(tree);
        catchWalker.Visit(root);
        Print.Line($"빈 catch 블록: {catchWalker.EmptyCatchLines.Count}개");
        foreach (int line in catchWalker.EmptyCatchLines)
            Print.Line($"  → Line {line}");

        // 리터럴 값 수집
        var litWalker = new LiteralCollector();
        litWalker.Visit(root);
        Print.Line($"리터럴 종류: 정수={litWalker.Integers.Count}  문자열={litWalker.Strings.Count}");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowSyntaxRewriter()
    {
        Print.Section("1-3. CSharpSyntaxRewriter — AST 변환");

        SyntaxTree tree = CSharpSyntaxTree.ParseText(SampleCode);
        var root = (CompilationUnitSyntax)tree.GetRoot();

        // 메서드 이름에 접두사 추가
        var prefixer = new PrefixMethodRewriter("Legacy_");
        var newRoot  = (CompilationUnitSyntax)prefixer.Visit(root);
        var renamed  = newRoot.DescendantNodes().OfType<MethodDeclarationSyntax>()
                              .Select(m => m.Identifier.Text).ToList();
        Print.Items(renamed, "이름 변환 후: ");

        // NormalizeWhitespace — 공백 정규화
        string normalized = newRoot.NormalizeWhitespace().ToFullString();
        Print.Line($"정규화 코드 길이: {normalized.Length}자");

        // void → 제거하는 Rewriter (예: public void → public)
        var publicVoidRemover = new VoidModifierRewriter();
        var changed = publicVoidRemover.Visit(root);
        int removedCount = publicVoidRemover.RemovedCount;
        Print.Line($"반환형 제거 시뮬레이션(void 메서드): {removedCount}개 처리");

        // 문자열 리터럴 중복 카운터
        const string dupCode = """
            class Foo {
                void Bar() { var a = "hello"; var b = "hello"; var c = "world"; }
            }
            """;
        var dupRoot    = CSharpSyntaxTree.ParseText(dupCode).GetRoot();
        var dupCounter = new StringLiteralCounter();
        dupCounter.Visit(dupRoot);
        foreach (var (s, cnt) in dupCounter.Duplicates)
            Print.Line($"  중복 문자열 \"{s}\" × {cnt}");

        // WithAdditionalAnnotations: 노드에 메타 태그 부착
        var method0 = root.DescendantNodes().OfType<MethodDeclarationSyntax>().First();
        var annotation = new SyntaxAnnotation("review", "performance");
        var annotated  = root.ReplaceNode(method0, method0.WithAdditionalAnnotations(annotation));
        var found      = annotated.GetAnnotatedNodes("review").FirstOrDefault();
        Print.Line($"어노테이션 복원: {found?.Kind()} [{found?.GetAnnotations("review").First().Data}]");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowTriviaAnalysis()
    {
        Print.Section("1-4. SyntaxTrivia — 주석 · 공백 · 전처리 지시문");

        SyntaxTree tree = CSharpSyntaxTree.ParseText(SampleCode);
        var root = (CompilationUnitSyntax)tree.GetRoot();
        var allTrivia = root.DescendantTrivia().ToList();

        var singleComments = allTrivia.Where(t => t.IsKind(SyntaxKind.SingleLineCommentTrivia)).ToList();
        var xmlDocs        = allTrivia.Where(t => t.IsKind(SyntaxKind.SingleLineDocumentationCommentTrivia)).ToList();
        var whitespaces    = allTrivia.Where(t => t.IsKind(SyntaxKind.WhitespaceTrivia)).ToList();
        var newlines       = allTrivia.Where(t => t.IsKind(SyntaxKind.EndOfLineTrivia)).ToList();

        Print.Line($"단일행 주석:   {singleComments.Count}개");
        Print.Line($"XML 문서 주석: {xmlDocs.Count}개");
        Print.Line($"공백 trivia:   {whitespaces.Count}개");
        Print.Line($"줄바꿈 trivia: {newlines.Count}개");

        // TODO 주석 탐지
        var todos = singleComments
            .Where(t => t.ToString().Contains("TODO", StringComparison.OrdinalIgnoreCase))
            .ToList();
        Print.Line($"TODO 주석 {todos.Count}개:");
        foreach (var t in todos)
        {
            var pos = tree.GetLineSpan(t.Span).StartLinePosition;
            Print.Line($"  Line {pos.Line + 1}: {t.ToString().Trim()}");
        }

        // XML 문서 파싱
        foreach (var doc in xmlDocs)
        {
            var docNode  = doc.GetStructure();
            var summary  = docNode?.DescendantNodes().OfType<XmlElementSyntax>()
                                   .FirstOrDefault(e => e.StartTag.Name.ToString() == "summary");
            if (summary is not null)
                Print.Line($"  XML summary: {summary.Content.ToString().Trim()}");
        }

        // SyntaxToken 레벨: 특정 토큰의 leading/trailing trivia
        var firstMethod = root.DescendantNodes().OfType<MethodDeclarationSyntax>().First();
        var nameToken   = firstMethod.Identifier;
        Print.Line($"메서드 토큰 '{nameToken.Text}'  leading={nameToken.LeadingTrivia.Count}  trailing={nameToken.TrailingTrivia.Count}");

        // DisabledText 등 전처리 지시문 (없으면 0)
        var directives = root.DescendantTrivia().Where(t => t.IsDirective).ToList();
        Print.Line($"전처리 지시문: {directives.Count}개");

        // SyntaxKind 값 확인
        Print.Line($"SyntaxKind 예: PublicKeyword={SyntaxKind.PublicKeyword} ({(int)SyntaxKind.PublicKeyword})");
    }
}

// ── Walker / Rewriter 구현 ────────────────────────────────────────────────────

sealed class IdentifierCollector : CSharpSyntaxWalker
{
    public List<string> Identifiers { get; } = new();
    public override void VisitIdentifierName(IdentifierNameSyntax node)
    {
        Identifiers.Add(node.Identifier.Text);
        base.VisitIdentifierName(node);
    }
}

sealed record MethodMeta(string Name, int ParamCount, string ReturnType, int LineCount, bool IsExpression);

sealed class MethodAnalysisWalker(SyntaxTree tree) : CSharpSyntaxWalker
{
    public List<MethodMeta> Methods { get; } = new();
    public override void VisitMethodDeclaration(MethodDeclarationSyntax node)
    {
        var span  = tree.GetLineSpan(node.Span);
        int lines = span.EndLinePosition.Line - span.StartLinePosition.Line + 1;
        Methods.Add(new MethodMeta(
            node.Identifier.Text,
            node.ParameterList.Parameters.Count,
            node.ReturnType.ToString(),
            lines,
            node.ExpressionBody is not null));
        base.VisitMethodDeclaration(node);
    }
}

sealed class EmptyCatchWalker(SyntaxTree tree) : CSharpSyntaxWalker
{
    public List<int> EmptyCatchLines { get; } = new();
    public override void VisitCatchClause(CatchClauseSyntax node)
    {
        if (node.Block.Statements.Count == 0)
            EmptyCatchLines.Add(tree.GetLineSpan(node.Span).StartLinePosition.Line + 1);
        base.VisitCatchClause(node);
    }
}

sealed class LiteralCollector : CSharpSyntaxWalker
{
    public List<int>    Integers { get; } = new();
    public List<string> Strings  { get; } = new();
    public override void VisitLiteralExpression(LiteralExpressionSyntax node)
    {
        if (node.IsKind(SyntaxKind.NumericLiteralExpression) && node.Token.Value is int i)
            Integers.Add(i);
        else if (node.IsKind(SyntaxKind.StringLiteralExpression))
            Strings.Add(node.Token.ValueText);
        base.VisitLiteralExpression(node);
    }
}

sealed class PrefixMethodRewriter(string prefix) : CSharpSyntaxRewriter
{
    public override SyntaxNode? VisitMethodDeclaration(MethodDeclarationSyntax node)
    {
        var newId = SyntaxFactory.Identifier(prefix + node.Identifier.Text)
                                 .WithTriviaFrom(node.Identifier);
        return base.VisitMethodDeclaration(node.WithIdentifier(newId));
    }
}

sealed class VoidModifierRewriter : CSharpSyntaxRewriter
{
    public int RemovedCount { get; private set; }
    public override SyntaxNode? VisitMethodDeclaration(MethodDeclarationSyntax node)
    {
        if (node.ReturnType is PredefinedTypeSyntax pt && pt.Keyword.IsKind(SyntaxKind.VoidKeyword))
            RemovedCount++;
        return base.VisitMethodDeclaration(node);
    }
}

sealed class StringLiteralCounter : CSharpSyntaxWalker
{
    private readonly Dictionary<string, int> _counts = new();
    public Dictionary<string, int> Duplicates { get; } = new();
    public override void VisitLiteralExpression(LiteralExpressionSyntax node)
    {
        if (node.IsKind(SyntaxKind.StringLiteralExpression))
        {
            string v = node.Token.ValueText;
            _counts.TryGetValue(v, out int cnt);
            _counts[v] = ++cnt;
            if (cnt > 1) Duplicates[v] = cnt;
        }
        base.VisitLiteralExpression(node);
    }
}
