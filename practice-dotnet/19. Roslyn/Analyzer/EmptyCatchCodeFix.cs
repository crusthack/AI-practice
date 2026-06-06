using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CodeActions;
using Microsoft.CodeAnalysis.CodeFixes;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using System;
using System.Collections.Immutable;
using System.Composition;
using System.Threading;
using System.Threading.Tasks;

namespace RoslynAnalyzer;

/// <summary>
/// EMPTYCAT001 진단에 대한 CodeFix: 빈 catch 블록에 throw; 문을 삽입합니다.
/// </summary>
[ExportCodeFixProvider(LanguageNames.CSharp, Name = nameof(EmptyCatchCodeFix))]
[Shared]
public sealed class EmptyCatchCodeFix : CodeFixProvider
{
    public override ImmutableArray<string> FixableDiagnosticIds =>
        [EmptyCatchAnalyzer.DiagnosticId];

    public override FixAllProvider? GetFixAllProvider() =>
        WellKnownFixAllProviders.BatchFixer;

    public override async Task RegisterCodeFixesAsync(CodeFixContext context)
    {
        var root = await context.Document.GetSyntaxRootAsync(context.CancellationToken);
        if (root is null) return;

        var node = root.FindNode(context.Diagnostics[0].Location.SourceSpan);
        if (node is not CatchClauseSyntax catchClause) return;

        context.RegisterCodeFix(
            CodeAction.Create(
                title: "throw; 재던지기로 채우기",
                createChangedDocument: ct => AddThrowAsync(context.Document, catchClause, ct),
                equivalenceKey: nameof(EmptyCatchCodeFix)),
            context.Diagnostics[0]);
    }

    private static async Task<Document> AddThrowAsync(
        Document document,
        CatchClauseSyntax catchClause,
        CancellationToken ct)
    {
        var root = await document.GetSyntaxRootAsync(ct);
        if (root is null) return document;

        var throwStmt = SyntaxFactory.ThrowStatement()
            .WithSemicolonToken(SyntaxFactory.Token(SyntaxKind.SemicolonToken))
            .WithLeadingTrivia(SyntaxFactory.Whitespace("                "))
            .WithTrailingTrivia(SyntaxFactory.EndOfLine("\n"));

        var newBlock = catchClause.Block.AddStatements(throwStmt);
        var newRoot  = root.ReplaceNode(catchClause.Block, newBlock);
        return document.WithSyntaxRoot(newRoot);
    }
}
