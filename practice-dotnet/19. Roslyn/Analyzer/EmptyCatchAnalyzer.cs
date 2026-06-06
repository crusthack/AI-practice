using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Diagnostics;
using System.Collections.Immutable;

namespace RoslynAnalyzer;

/// <summary>
/// 빈 catch 블록을 탐지하는 Roslyn DiagnosticAnalyzer.
/// Consumer 프로젝트에서 OutputItemType="Analyzer" 로 참조하면 빌드 시 자동 실행됩니다.
/// </summary>
[DiagnosticAnalyzer(LanguageNames.CSharp)]
public sealed class EmptyCatchAnalyzer : DiagnosticAnalyzer
{
    public const string DiagnosticId = "EMPTYCAT001";

    private static readonly DiagnosticDescriptor Rule = new(
        id: DiagnosticId,
        title: "빈 catch 블록",
        messageFormat: "빈 catch 블록이 예외를 무시합니다: 최소 로그 또는 throw를 추가하세요",
        category: "BestPractice",
        defaultSeverity: DiagnosticSeverity.Warning,
        isEnabledByDefault: true,
        description: "빈 catch 블록은 예외를 무음으로 삼켜 디버깅을 어렵게 합니다.");

    public override ImmutableArray<DiagnosticDescriptor> SupportedDiagnostics => [Rule];

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
            ctx.ReportDiagnostic(Diagnostic.Create(Rule, clause.GetLocation()));
    }
}
