using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Scripting;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Formatting;
using Microsoft.CodeAnalysis.Scripting;
using Microsoft.CodeAnalysis.Text;

namespace Roslyn.Demos;

static class D5_Workspace
{
    public static async Task Run()
    {
        Print.Header("5. Workspace · Formatter · Scripting");
        ShowNormalizeWhitespace();
        await ShowWorkspaceAsync();
        await ShowScriptingAsync();
    }

    // ─────────────────────────────────────────────────────────────────────────
    static void ShowNormalizeWhitespace()
    {
        Print.Section("5-1. NormalizeWhitespace — 경량 포매팅");

        const string ugly = "class Foo{void Bar(){int x=1+2;Console.WriteLine(x);}}";
        var tree    = CSharpSyntaxTree.ParseText(ugly);
        var root    = tree.GetRoot();
        var pretty  = root.NormalizeWhitespace().ToFullString();

        Print.Line($"원본 길이: {ugly.Length}자  정규화 후: {pretty.Length}자");
        foreach (var line in pretty.Split('\n').Take(8))
            Console.WriteLine("    " + line.TrimEnd());

        // ToFullString vs ToString — trivia 포함 여부 차이
        var method = tree.GetRoot().DescendantNodes().OfType<MethodDeclarationSyntax>().FirstOrDefault();
        if (method is not null)
        {
            Print.Line($"ToString    : \"{method.ToString()[..Math.Min(40, method.ToString().Length)]}...\"");
            Print.Line($"ToFullString: \"{method.ToFullString()[..Math.Min(40, method.ToFullString().Length)]}...\"");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowWorkspaceAsync()
    {
        Print.Section("5-2. AdhocWorkspace — 프로젝트·문서·솔루션 관리");

        const string code = """
            using System;

            namespace Demo
            {
            class   MyService  {
            public  void   DoWork(  )  {
            var x=1+2;
            Console.WriteLine(x);
            }
            }
            }
            """;

        using var workspace = new AdhocWorkspace();

        var projectId = ProjectId.CreateNewId();
        var docId     = DocumentId.CreateNewId(projectId);

        // 기본 메타데이터 참조 (Console 등 사용을 위해)
        var trusted = ((string?)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES"))
            ?.Split(Path.PathSeparator) ?? [];
        var needed = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            { "System.Private.CoreLib.dll", "System.Runtime.dll", "System.Console.dll" };
        var metaRefs = trusted.Where(p => needed.Contains(Path.GetFileName(p)))
                              .Select(p => (MetadataReference)MetadataReference.CreateFromFile(p));

        // Solution 빌드: Project + Document 추가
        var solution = workspace.CurrentSolution
            .AddProject(projectId, "DemoProject", "DemoProject", LanguageNames.CSharp)
            .WithProjectCompilationOptions(projectId,
                new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary))
            .WithProjectParseOptions(projectId,
                CSharpParseOptions.Default.WithLanguageVersion(LanguageVersion.Latest))
            .AddMetadataReferences(projectId, metaRefs)
            .AddDocument(docId, "MyService.cs", SourceText.From(code));

        workspace.TryApplyChanges(solution);
        Print.Line($"프로젝트: {workspace.CurrentSolution.Projects.First().Name}");
        Print.Line($"문서 수:  {workspace.CurrentSolution.Projects.First().Documents.Count()}");

        // Formatter.FormatAsync — 워크스페이스 기반 포매팅
        var document       = workspace.CurrentSolution.GetDocument(docId)!;
        var formattedDoc   = await Formatter.FormatAsync(document);
        var formattedText  = await formattedDoc.GetTextAsync();

        Print.Line($"원본 라인: {code.Split('\n').Length}  포맷 후: {formattedText.Lines.Count}");
        foreach (var line in formattedText.Lines.Take(10).Select(l => l.ToString()))
            Console.WriteLine("    " + line);

        // 문서 수정: 변경사항 적용
        var newCode = formattedText.ToString().Replace("DoWork", "Execute");
        var newDoc  = document.WithText(SourceText.From(newCode));
        workspace.TryApplyChanges(newDoc.Project.Solution);
        Print.Line($"메서드 이름 변경 적용: {workspace.CurrentSolution.GetDocument(docId) is not null}");

        // Solution 정보
        var sol = workspace.CurrentSolution;
        Print.Line($"SolutionId: {sol.Id}");
        Print.Line($"ProjectIds: {sol.ProjectIds.Count}개");

        // Compilation 획득
        var proj = workspace.CurrentSolution.GetProject(projectId)!;
        var comp = await proj.GetCompilationAsync();
        Print.Line($"Compilation 오류: {comp?.GetDiagnostics().Count(d => d.Severity == DiagnosticSeverity.Error)}개");
    }

    // ─────────────────────────────────────────────────────────────────────────
    static async Task ShowScriptingAsync()
    {
        Print.Section("5-3. CSharpScript — 동적 C# 스크립트 실행");

        // 단순 평가
        int    r1 = await CSharpScript.EvaluateAsync<int>("1 + 2 + 3");
        double r2 = await CSharpScript.EvaluateAsync<double>(
            "Math.Sqrt(2.0)",
            ScriptOptions.Default.AddImports("System"));
        Print.Line($"1+2+3 = {r1}");
        Print.Line($"sqrt(2) = {r2:F6}");

        // ScriptOptions: 참조 + using 추가
        var opts = ScriptOptions.Default
            .AddImports("System", "System.Collections.Generic", "System.Linq")
            .AddReferences(
                typeof(object).Assembly,
                typeof(System.Linq.Enumerable).Assembly);

        string linqScript = "Enumerable.Range(1, 5).Select(x => x * x).ToList()";
        var squares = await CSharpScript.EvaluateAsync<List<int>>(linqScript, opts);
        Print.Items(squares, "squares: ");

        // Stateful 스크립트 세션 — 상태가 이어짐
        var state1 = await CSharpScript.RunAsync("int x = 42;", opts);
        var state2 = await state1.ContinueWithAsync("int y = x * 2;");
        var state3 = await state2.ContinueWithAsync<int>("x + y");
        Print.Line($"stateful 결과: x={42} y={84} x+y={state3.ReturnValue}");

        // 변수 목록 확인
        Print.Line("스크립트 변수:");
        foreach (var v in state3.Variables)
            Print.Line($"  {v.Type.Name} {v.Name} = {v.Value}");

        // Globals: 외부 객체를 스크립트에 주입
        var globals = new ScriptGlobals { Base = 10, Multiplier = 3 };
        var withGlobals = await CSharpScript.EvaluateAsync<int>(
            "Base * Multiplier + Base",
            ScriptOptions.Default,
            globals: globals);
        Print.Line($"globals 결과: {globals.Base} * {globals.Multiplier} + {globals.Base} = {withGlobals}");

        // 컴파일 오류 처리
        try
        {
            await CSharpScript.EvaluateAsync<int>("int x = \"not an int\";");
        }
        catch (CompilationErrorException ex)
        {
            Print.Line($"컴파일 오류 잡힘: {ex.Diagnostics.Length}개 → {ex.Diagnostics[0].Id}");
        }

        // Script<T> 객체로 반복 실행 (컴파일 1회, 실행 N회)
        var script = CSharpScript.Create<double>(
            "Math.PI * radius * radius",
            ScriptOptions.Default.AddImports("System"),
            globalsType: typeof(CircleGlobals));

        var compiled = script.Compile();
        Print.Line($"Script 컴파일 오류: {compiled.Length}개");

        foreach (double radius in new[] { 1.0, 2.0, 5.0 })
        {
            var result = await script.RunAsync(new CircleGlobals { radius = radius });
            Print.Line($"  r={radius} → area={result.ReturnValue:F4}");
        }
    }
}

public class ScriptGlobals
{
    public int Base       { get; set; }
    public int Multiplier { get; set; }
}

public class CircleGlobals
{
    public double radius;
}
